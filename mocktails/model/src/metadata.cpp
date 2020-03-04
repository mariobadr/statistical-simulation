#include "mocktails/metadata.hpp"

#include <stm/metadata.hpp>
#include <hrd/metadata.hpp>

#include "mocktails.pb.h"

namespace mocktails {

template <typename Type>
void convert(std::int64_t input, Type &output)
{
  output = static_cast<Type>(input);
}

void convert(std::int64_t input, operation &output)
{
  if(input == 0) {
    output = operation::read;
  } else {
    output = operation::write;
  }
}

template <typename Type>
sequence_model<Type> deserialize(SequenceModel const &model)
{
  sequence_model<Type> sm{};

  convert(model.initial_state(), sm.initial_state);
  sm.current_state = sm.initial_state;

  if(model.has_transition_matrix()) {
    auto const &tm = model.transition_matrix();

    for(int s = 0; s < tm.transition_size(); ++s) {
      auto const &t = tm.transition(s);

      Type from, to;
      convert(t.from(), from);
      convert(t.to(), to);

      sm.transitions[from][to] = t.count();
    }
  }

  return sm;
}

template <typename Type>
void serialize(sequence_model<Type> const &src_model, SequenceModel *dst_model)
{
  auto const initial_state = static_cast<std::int64_t>(src_model.initial_state);
  dst_model->set_initial_state(initial_state);

  if(!src_model.transitions.empty()) {
    auto proto_matrix = dst_model->mutable_transition_matrix();

    for(auto const &from_pair : src_model.transitions) {
      for(auto const &to_pair : from_pair.second) {
        auto transition = proto_matrix->add_transition();

        auto const from = static_cast<std::int64_t>(from_pair.first);
        auto const to = static_cast<std::int64_t>(to_pair.first);

        transition->set_from(from);
        transition->set_to(to);
        transition->set_count(to_pair.second);
      }
    }
  }
}

std::unique_ptr<profile> read_stm(ioproto::istream &stream, ProfileHeader const &header)
{
  auto p = std::make_unique<profile>(0, model_type::stm);

  for(std::uint64_t i = 0; i < header.model_count(); ++i) {
    std::uint32_t node_id;
    model<stm::profile> leaf;

    {
      ModelHeader mh;
      if(!stream.read(&mh)) {
        throw std::runtime_error("Could not read an expected ModelHeader.");
      }

      node_id = mh.node_id();

      leaf.start_time = mh.start_time();
      leaf.size_model = deserialize<std::uint64_t>(mh.sizes());
      leaf.time_model = deserialize<std::uint64_t>(mh.delta_times());
    }

    leaf.underlying_model = stm::read(stream);
    p->stm_leaves.emplace(node_id, std::move(leaf));
  }

  return p;
}

std::unique_ptr<profile> read_hrd(ioproto::istream &stream, ProfileHeader const &header)
{
  auto p = std::make_unique<profile>(0, model_type::hrd);

  for(std::uint64_t i = 0; i < header.model_count(); ++i) {
    std::uint32_t node_id;
    model<hrd::profile> leaf;

    {
      ModelHeader mh;
      if(!stream.read(&mh)) {
        throw std::runtime_error("Could not read an expected ModelHeader.");
      }

      node_id = mh.node_id();

      leaf.start_time = mh.start_time();
      leaf.size_model = deserialize<std::uint64_t>(mh.sizes());
      leaf.time_model = deserialize<std::uint64_t>(mh.delta_times());
    }

    leaf.underlying_model = std::make_unique<hrd::profile>(hrd::read(stream, &leaf.request_count));
    p->hrd_leaves.emplace(node_id, std::move(leaf));
  }

  return p;
}

std::unique_ptr<profile> read_mocktails(ioproto::istream &stream, ProfileHeader const &header)
{
  auto p = std::make_unique<profile>(0, model_type::mocktails);

  for(std::uint64_t i = 0; i < header.model_count(); ++i) {
    std::uint32_t node_id;
    model<simple_model> leaf;

    {
      ModelHeader mh;
      if(!stream.read(&mh)) {
        throw std::runtime_error("Could not read an expected ModelHeader.");
      }

      node_id = mh.node_id();

      leaf.start_time = mh.start_time();
      leaf.size_model = deserialize<std::uint64_t>(mh.sizes());
      leaf.time_model = deserialize<std::uint64_t>(mh.delta_times());
    }

    {
      Model m;
      if(!stream.read(&m)) {
        throw std::runtime_error("Could not read an expected Model.");
      }

      leaf.underlying_model = std::make_unique<simple_model>();

      leaf.request_count = m.request_count();
      leaf.underlying_model->start_address = m.footprint().start_address();
      leaf.underlying_model->footprint.start = m.footprint().min_address();
      leaf.underlying_model->footprint.end = m.footprint().max_address();
      leaf.underlying_model->operation_model = deserialize<operation>(m.operations());
      leaf.underlying_model->stride_model = deserialize<stride_t>(m.strides());
    }

    p->simple_leaves.emplace(node_id, std::move(leaf));
  }

  return p;
}

std::unique_ptr<profile> read(ioproto::istream &stream)
{
  ProfileHeader header;
  if(!stream.read(&header)) {
    return nullptr;
  }

  if(header.model_type() == ProfileHeader_ModelType_MOCKTAILS) {
    return read_mocktails(stream, header);
  } else if(header.model_type() == ProfileHeader_ModelType_REUSE_DISTANCE) {
    return read_hrd(stream, header);
  } else if(header.model_type() == ProfileHeader_ModelType_STM) {
    return read_stm(stream, header);
  } else {
    return nullptr;
  }
}

template <typename Type>
void write_model_header(ioproto::ofstream &stream, std::uint32_t id, model<Type> const &leaf)
{
  ModelHeader mh;
  mh.set_node_id(id);
  mh.set_start_time(leaf.start_time);

  serialize(leaf.size_model, mh.mutable_sizes());
  serialize(leaf.time_model, mh.mutable_delta_times());

  stream.write(mh);
}

ProfileHeader_ModelType convert(model_type type)
{
  if(type == model_type::mocktails) {
    return ProfileHeader_ModelType::ProfileHeader_ModelType_MOCKTAILS;
  } else if(type == model_type::stm) {
    return ProfileHeader_ModelType::ProfileHeader_ModelType_STM;
  } else if(type == model_type::hrd) {
    return ProfileHeader_ModelType::ProfileHeader_ModelType_REUSE_DISTANCE;
  }

  throw std::runtime_error("Unknown (or not supported) model type.");
}

void append(ioproto::ofstream &stream, profile const &p)
{
  {
    ProfileHeader header;
    header.set_model_count(p.model_count);
    header.set_model_type(convert(p.type));

    stream.write(header);
  }

  if(p.type == model_type::mocktails) {
    for(auto const &pair : p.simple_leaves) {
      auto &leaf = pair.second;
      write_model_header(stream, pair.first, leaf);

      Model m;
      m.set_request_count(leaf.request_count);

      Footprint *fp = m.mutable_footprint();
      fp->set_start_address(leaf.underlying_model->start_address);
      fp->set_min_address(leaf.underlying_model->footprint.start);
      fp->set_max_address(leaf.underlying_model->footprint.end);

      serialize(leaf.underlying_model->operation_model, m.mutable_operations());
      serialize(leaf.underlying_model->stride_model, m.mutable_strides());

      stream.write(m);
    }
  } else if(p.type == model_type::stm) {
    for(auto const &pair : p.stm_leaves) {
      auto &leaf = pair.second;
      write_model_header(stream, pair.first, leaf);
      stm::append(stream, *leaf.underlying_model);
    }
  } else if(p.type == model_type::hrd) {
    for(auto const &pair : p.hrd_leaves) {
      auto &leaf = pair.second;
      write_model_header(stream, pair.first, leaf);
      hrd::append(stream, *leaf.underlying_model);
    }
  }
}
} // namespace mocktails