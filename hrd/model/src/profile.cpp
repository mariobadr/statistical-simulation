#include "hrd/profile.hpp"

#include <cmath>
#include <limits>
#include <hrd/profile.hpp>

namespace hrd {

inline std::uint64_t calculate_block(std::uint64_t address, std::uint64_t block_size)
{
  auto const block = std::floor(address / block_size);

  return static_cast<std::uint64_t>(block);
}

profile::profile(std::vector<std::uint64_t> levels)
    : layers(std::move(levels))
    , reuse_model(layers.size())
    , min_address(std::numeric_limits<std::uint64_t>::max())
    , m_info(layers.size())
{
  for(std::size_t i = 0; i < MEMORY_STATE_COUNT; i++) {
    for(std::size_t j = 0; j < OPERATION_COUNT; j++) {
      ops_model[i][j] = 0;
    }
  }
}

void profile::update(std::uint64_t address, operation op)
{
  min_address = std::min(address, min_address);
  max_address = std::max(address, max_address);

  model_reuse(address);
  model_operation(address, op);
}

void profile::model_reuse(std::uint64_t address)
{
  constexpr double INF = std::numeric_limits<double>::infinity();

  double distance = INF;
  std::size_t layer = 0;

  while(distance == INF && layer < layers.size()) {
    // Calculate the block for this address.
    auto const block = calculate_block(address, layers[layer]);

    distance = reuse_distance::compute_distance(m_info[layer], block);

    // Update the histogram for the given layer.
    reuse_model[layer][distance]++;

    ++layer;
  }

  // Update the remaining layers, ignoring the computed reuse distance.
  for(layer = 0; layer < layers.size(); ++layer) {
    auto const block = calculate_block(address, layers[layer]);
    reuse_distance::update(m_info[layer], block, m_time);
  }

  m_time++;
}

void profile::model_operation(std::uint64_t address, operation op)
{
  auto &current_state = m_states[address];

  auto const state_index = static_cast<std::size_t>(current_state);
  auto const op_index = static_cast<std::size_t>(op);
  ops_model[state_index][op_index]++;

  // Transition to appropriate state based on current state and the operation.
  if(current_state == memory_state::invalid && op == operation::read) {
    current_state = memory_state::clean;
  } else if(op == operation::write) {
    current_state = memory_state::dirty;
  }
}

std::size_t profile::unique_addresses() const
{
  return m_states.size();
}

std::uint64_t profile::count() const
{
  // The total number of requests modelled is equivalent to the logical time.
  return m_time;
}

} // namespace hrd