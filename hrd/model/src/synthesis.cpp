#include "hrd/synthesis.hpp"

#include <algorithm>
#include <cassert>
#include <hrd/synthesis.hpp>

namespace hrd {

constexpr double INF = std::numeric_limits<double>::infinity();

inline std::uint64_t calculate_block(std::uint64_t address, std::uint64_t block_size)
{
  auto const block = std::floor(address / block_size);
  return static_cast<std::uint64_t>(block);
}

std::uint64_t
uniform(std::mt19937 &rng, std::uint64_t min, std::uint64_t max, std::uint64_t multiple = 1)
{
  std::uniform_int_distribution<std::uint64_t> distribution(0, (max - min) / multiple - 1);
  auto const generated = distribution(rng);

  auto const result = min + (generated * multiple);
  assert(result >= min);
  assert(result < max);

  return result;
}

double random_reuse(std::mt19937 &rng, synthesiser::histogram_t const &hist, std::size_t stack_size)
{
  auto const max_distance = static_cast<double>(stack_size);

  // Find the maximum index based on the maximum distance allowed.
  std::size_t max_index = 1;
  while(max_index < hist.distances.size() && hist.distances[max_index] < max_distance) {
    max_index++;
  }

  // The first index is the count for infinity.
  auto const begin = hist.counts.begin();
  // An iterator that is one past the "end".
  auto const end = hist.counts.begin() + static_cast<int>(max_index);

  // Create a distribution based on the reuse_counts.
  std::discrete_distribution<std::size_t> distribution(begin, end);
  auto const index = distribution(rng);

  auto const distance = hist.distances[index];
  assert(distance == INF || distance < max_distance);

  return distance;
}

std::uint64_t read_history(synthesiser::info_t const &info, double distance)
{
  // Convert the distance to an offset from the beginning of the stack.
  auto const offset = static_cast<std::size_t>(distance);

  if(offset < info.tree.size()) {
    auto node = info.tree.most_recently_used();
    for(std::size_t i = 0; i < offset; i++) {
      node = info.tree.predecessor(node);
    }
    return node->address;
  }

  // The requested distance exceeded the stack size, return the least recently used block.
  return info.tree.least_recently_used()->address;
}

void update_history(synthesiser::info_t &info, std::uint64_t block)
{
  reuse_distance::update(info.tree, block, info.time);
  info.time++;
}

bool new_unique(std::mt19937 &rng,
    std::uint64_t min,
    std::uint64_t max,
    std::uint64_t coarsest_block_size,
    std::uint64_t target_block_size,
    addresses_per_block const &coarsest_blocks,
    std::uint64_t &unique_address)
{
  std::uint64_t block;

  auto const max_blocks = (max - min) / coarsest_block_size;
  if(coarsest_blocks.size() >= max_blocks) {
    return false;
  }

  do {
    unique_address = uniform(rng, min, max, target_block_size);
    block = calculate_block(unique_address, coarsest_block_size);
  } while(coarsest_blocks.find(block) != coarsest_blocks.end());

  return true;
}

bool new_unique(std::mt19937 &rng,
    std::uint64_t block,
    std::uint64_t parent_block_size,
    std::uint64_t target_block_size,
    std::set<std::uint64_t> const &generated,
    std::uint64_t &unique_address)
{
  std::uint64_t const min = block * parent_block_size;
  std::uint64_t const max = min + parent_block_size;

  auto const max_blocks = parent_block_size / target_block_size;
  if(generated.size() >= max_blocks) {
    return false;
  }

  do {
    unique_address = uniform(rng, min, max, target_block_size);
  } while(generated.find(unique_address) != generated.end());

  return true;
}

bool try_generate_address(std::mt19937 &rng,
    std::vector<synthesiser::layer_t> const &layers,
    std::uint64_t min_address,
    std::uint64_t max_address,
    std::uint64_t &address)
{
  std::size_t const Nl = layers.size();

  double distance = INF;
  std::size_t l = 0;

  while(distance == INF && l < Nl) {
    if(!layers[l].info.tree.empty()) {
      distance = random_reuse(rng, layers[l].hist, layers[l].info.tree.size());
    }

    l = l + 1;
  }

  bool success;
  if(distance == INF) {
    // All layers of the hierarchy missed.
    // Generate a new and unique address in a new and unique block.
    success = new_unique(rng, min_address, max_address, layers.back().block_size,
        layers.front().block_size, layers.back().generated, address);
  } else {
    // One of the layers of the hierarchy is reusing a block.
    auto const block = read_history(layers[l - 1].info, distance);

    if(l - 1 == 0) {
      // The lowest level of the hierarchy is reusing a block, no need to generate a finer address.
      address = block * layers[l - 1].block_size;
      success = true;
    } else {
      auto it = layers[l - 1].generated.find(block);
      assert(it != layers[l - 1].generated.end());

      // Generate a new and unique address within some block.
      success = new_unique(
          rng, block, layers[l - 1].block_size, layers[l - 2].block_size, it->second, address);
    }
  }

  return success;
}

std::uint64_t generate_address(std::mt19937 &rng,
    std::vector<synthesiser::layer_t> &layers,
    std::uint64_t min_address,
    std::uint64_t max_address)
{
  std::uint64_t address;
  while(!try_generate_address(rng, layers, min_address, max_address, address)) {
  }

  for(auto &layer : layers) {
    std::uint64_t const block = calculate_block(address, layer.block_size);
    update_history(layer.info, block);

    // Track the addresses being generated for each block.
    layer.generated[block].insert(address);
  }

  return address;
}

void update_state(memory_state &state, operation op)
{
  if(state == memory_state::invalid && op == operation::read) {
    state = memory_state::clean;
  } else if(op == operation::write) {
    state = memory_state::dirty;
  }
}

operation generate_operation(std::mt19937 &rng, double reads, double writes)
{
  std::discrete_distribution<> distribution({reads, writes});

  if(distribution(rng) == 0) {
    return operation::read;
  }

  return operation::write;
}

synthesiser::synthesiser(profile const &p)
    : m_min_address(p.min_address)
    , m_max_address(p.max_address)
    , m_layers(p.layers.size())
    , m_op_hist(p.ops_model)
{
  for(std::size_t i = 0; i < p.layers.size(); ++i) {
    m_layers[i].block_size = p.layers[i];
    m_layers[i].hist.distances.push_back(INF);
    m_layers[i].hist.counts.push_back(p.reuse_model[i].at(INF));

    for(auto const &pair : p.reuse_model[i]) {
      if(pair.first != INF) {
        m_layers[i].hist.distances.push_back(pair.first);
        m_layers[i].hist.counts.push_back(pair.second);
      }
    }
  }
}

void synthesiser::generate_next_request(uint64_t &address, operation &op)
{
  static auto constexpr read_index = static_cast<std::size_t>(operation::read);
  static auto constexpr write_index = static_cast<std::size_t>(operation::write);

  address = generate_address(m_rng, m_layers, m_min_address, m_max_address);

  auto &state = m_states[address];
  auto const state_index = static_cast<std::size_t>(state);

  auto const read_count = static_cast<double>(m_op_hist[state_index][read_index]);
  auto const write_count = static_cast<double>(m_op_hist[state_index][write_index]);

  op = generate_operation(m_rng, read_count, write_count);
  update_state(state, op);
}
} // namespace hrd
