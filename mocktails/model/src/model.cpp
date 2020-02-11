#include "mocktails/model.hpp"

namespace mocktails {

template <typename ModelType>
model<ModelType> create_model(std::vector<request> const &requests)
{
  assert(!requests.empty());

  std::uint64_t last_time = 0;
  sequence<std::uint64_t> time_sequence{};
  sequence<std::uint64_t> size_sequence{};
  bool first_request = true;

  model<ModelType> m{};
  for(auto const &req : requests) {
    size_sequence.add(req.size);

    if(first_request) {
      first_request = false;
    } else {
      auto const delta_time = req.timestamp - last_time;
      time_sequence.add(delta_time);
    }

    last_time = req.timestamp;
  }

  m.request_count = requests.size();
  m.start_time = requests.front().timestamp;
  m.size_model = size_sequence.make_model();
  m.time_model = time_sequence.make_model();

  return m;
}

model<simple_model> create_simple_model(std::vector<request> const &requests)
{
  assert(!requests.empty());

  sequence<operation> op_sequence{};
  sequence<stride_t> stride_sequence{};
  address_t last_address = 0;
  bool first_request = true;

  auto m = create_model<simple_model>(requests);
  m.underlying_model = std::make_unique<simple_model>();

  for(auto const &req : requests) {
    op_sequence.add(req.op);

    if(first_request) {
      first_request = false;
    } else {
      auto const stride = calculate_stride(req.address, last_address);
      stride_sequence.add(stride);
    }

    m.underlying_model->footprint.start =
        std::min(m.underlying_model->footprint.start, req.address);
    m.underlying_model->footprint.end =
        std::max(m.underlying_model->footprint.end, req.address + req.size);

    last_address = req.address;
  }

  m.underlying_model->operation_model = op_sequence.make_model();
  m.underlying_model->stride_model = stride_sequence.make_model();

  m.request_count = requests.size();
  m.underlying_model->start_address = requests.front().address;

  return m;
}

stm::operation to_stm_operation(operation op)
{
  if(op == operation::read) {
    return stm::operation::read;
  }

  return stm::operation::write;
}

model<stm::profile> create_stm_model(std::vector<request> const &requests)
{
  auto m = create_model<stm::profile>(requests);

  // Generate an STM model for this partition (SDC is 32 rows, 2 columns. Stride depth is 8).
  m.underlying_model = std::make_unique<stm::profile>(stm::profile::parameters{32, 2, 8});

  for(auto const &r : requests) {
    m.underlying_model->update(r.address, to_stm_operation(r.op));
  }

  return m;
}

hrd::operation to_hrd_operation(operation op)
{
  if(op == operation::read) {
    return hrd::operation::read;
  }

  return hrd::operation::write;
}

model<hrd::profile> create_hrd_model(std::vector<request> const &requests)
{
  auto m = create_model<hrd::profile>(requests);

  // Generate a flat reuse distance model with a block size of 64 bytes.
  m.underlying_model = std::make_unique<hrd::profile>(std::vector<std::uint64_t>{64});

  for(auto const &r : requests) {
    m.underlying_model->update(r.address, to_hrd_operation(r.op));
  }

  return m;
}

} // namespace mocktails