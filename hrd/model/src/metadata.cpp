#include "hrd/metadata.hpp"

#include "hrd.pb.h"

namespace hrd {

profile read(ioproto::istream &stream, std::uint64_t *request_count)
{
  hrd::profile profile({});

  {
    hrd::Configuration proto_config;
    if(!stream.read(&proto_config)) {
      throw std::runtime_error("Could not read hierarchical configuration from file.");
    }

    for(int i = 0; i < proto_config.block_sizes_size(); i++) {
      profile.layers.push_back(proto_config.block_sizes(i));
    }
  }

  {
    hrd::GlobalData proto_global;
    if(!stream.read(&proto_global)) {
      throw std::runtime_error("Could not read global data from file.");
    }

    if(request_count != nullptr) {
      *request_count = proto_global.total_requests();
    }

    profile.min_address = proto_global.min_address();
    profile.max_address = proto_global.max_address();
  }

  {
    hrd::OperationsModel proto_ops;
    if(!stream.read(&proto_ops)) {
      throw std::runtime_error("Could not read operations model from file.");
    }

    for(int i = 0; i < proto_ops.reads_size(); i++) {
      auto const state = static_cast<std::size_t>(i);
      auto const read = static_cast<std::size_t>(hrd::operation::read);
      auto const write = static_cast<std::size_t>(hrd::operation::write);

      profile.ops_model[state][read] = proto_ops.reads(i);
      profile.ops_model[state][write] = proto_ops.writes(i);
    }
  }

  {
    hrd::ReuseModel proto_reuse;
    if(!stream.read(&proto_reuse)) {
      throw std::runtime_error("Could not read the reuse model from the file.");
    }

    auto const num_layers = static_cast<std::size_t>(proto_reuse.layer_size());
    profile.reuse_model.resize(num_layers);

    for(std::size_t layer = 0; layer < num_layers; layer++) {
      const hrd::ReuseModel_Histogram &histogram = proto_reuse.layer(static_cast<int>(layer));

      // Assume the distance and count arrays in the histogram are the same size.
      for(int i = 0; i < histogram.distance_size(); i++) {
        profile.reuse_model[layer][histogram.distance(i)] = histogram.count(i);
      }
    }
  }

  return profile;
}

void append(ioproto::ofstream &stream, const profile &p)
{
  {
    Configuration config;

    for(auto const &block_size : p.layers) {
      config.add_block_sizes(block_size);
    }

    stream.write(config);
  }

  {
    GlobalData global;

    global.set_total_requests(p.count());
    global.set_max_address(p.max_address);
    global.set_min_address(p.min_address);

    stream.write(global);
  }

  {
    OperationsModel ops;

    auto const read_index = static_cast<std::size_t>(operation::read);
    auto const write_index = static_cast<std::size_t>(operation::write);

    for(std::size_t state = 0; state < MEMORY_STATE_COUNT; state++) {
      ops.add_reads(p.ops_model[state][read_index]);
      ops.add_writes(p.ops_model[state][write_index]);
    }

    stream.write(ops);
  }

  {
    ReuseModel layers;

    for(auto const &l : p.reuse_model) {
      auto histogram = layers.add_layer();

      for(auto const &hist : l) {
        histogram->add_distance(hist.first);
        histogram->add_count(hist.second);
      }
    }

    stream.write(layers);
  }
}
} // namespace hrd