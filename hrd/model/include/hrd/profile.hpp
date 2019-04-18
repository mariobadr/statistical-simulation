#ifndef HRD_CLONING_PROFILE_HPP
#define HRD_CLONING_PROFILE_HPP

#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

#include <reuse-distance/olken.hpp>

#include <hrd/request-type.hpp>

namespace hrd {

/**
 * Captures the distribution of reuse distances.
 */
using reuse_histogram = std::map<double, std::uint64_t>;

/**
 * A module for building Hierarchical Reuse Distance models.
 */
class profile {
public:
  /**
   * Constructor.
   *
   * @param levels The block sizes to consider, assumed to be an ascending order.
   */
  explicit profile(std::vector<std::uint64_t> levels);

  /**
   * Update the reuse distance model and read/write model.
   *
   * @param address The address of the next access.
   * @param op The operation of the next access.
   */
  void update(std::uint64_t address, operation op);

  /**
   * @return The number of unique addresses modelled by the profile.
   */
  std::size_t unique_addresses() const;

  /**
   * @return The total number of requests modelled by the profile.
   */
  std::uint64_t count() const;

public:
  /** The block sizes per level of the hierarchy. */
  std::vector<std::uint64_t> layers;
  /** The reuse distance distributions per level of the hierarchy. */
  std::vector<reuse_histogram> reuse_model;
  /** The distributions of reads/writes for each memory_state of each address. */
  std::array<operation_histogram, 3> ops_model;
  /** The start of the address range profiled. */
  std::uint64_t min_address;
  /** The end of the address range profiled. */
  std::uint64_t max_address = 0;

private:
  void model_reuse(std::uint64_t address);

  void model_operation(std::uint64_t address, operation op);

private:
  // Logical time counter.
  std::uint64_t m_time = 0;
  // The reuse distance data structures per level of the hierarchy.
  std::vector<reuse_distance::olken_tree> m_info;
  // The current state of a unique address in memory.
  std::unordered_map<std::uint64_t, memory_state> m_states;
};

} // namespace hrd

#endif // HRD_CLONING_PROFILE_HPP
