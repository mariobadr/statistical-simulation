#ifndef HRD_CLONING_SYNTHESIS_HPP
#define HRD_CLONING_SYNTHESIS_HPP

#include <deque>
#include <map>
#include <set>
#include <random>

#include <hrd/profile.hpp>

namespace hrd {

using addresses_per_block = std::map<std::uint64_t, std::set<std::uint64_t>>;

class synthesiser {
public:
  explicit synthesiser(profile const &p);

  /**
   * Synthesize a request.
   *
   * @param address The synthesized address.
   * @param op The synthesized operation.
   */
  void generate_next_request(uint64_t &address, operation &op);

public:
  struct histogram_t {
    std::vector<double> distances;
    std::vector<std::uint64_t> counts;
  };

  struct info_t {
    reuse_distance::olken_tree tree;
    std::uint64_t time = 0;
  };

  struct layer_t {
    std::uint64_t block_size;
    histogram_t hist;
    info_t info;
    addresses_per_block generated;
  };

private:
  std::mt19937 m_rng;

  std::uint64_t m_min_address;
  std::uint64_t m_max_address;

  std::vector<layer_t> m_layers;

  std::array<operation_histogram, 3> m_op_hist;
  std::map<std::uint64_t, memory_state> m_states;
};

} // namespace hrd

#endif //HRD_CLONING_SYNTHESIS_HPP
