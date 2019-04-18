#ifndef STM_CLONING_PROFILE_HPP
#define STM_CLONING_PROFILE_HPP

#include "stm/stack-distance.hpp"
#include "stm/stride-pattern.hpp"

/**
 * A module for building Spatial and Temporal Memory Access models.
 */
namespace stm {

enum class operation { read, write };

/**
 * Spatial and Temporal Memory (STM) Access Model.
 */
class profile {
public:
  /**
   * Parameters that configure the stack distance and stride pattern tables.
   */
  struct parameters {
    /**
     * Number of rows in the stack distance profile.
     *
     * The default used in the paper is 128.
     */
    std::size_t num_rows = 128;

    /**
     * Number of columns in the stack distance profile.
     *
     * The default used in the paper is 2.
     */
    std::size_t num_cols = 2;

    /**
     * Depth of history to capture for stride patterns.
     *
     * The default used in the paper is 80.
     */
    std::size_t stride_depth = 80;
  };

  /**
   * Constructor.
   *
   * @param p The STM parameters that configure the tables.
   */
  explicit profile(parameters const &p);

  /**
   * Update the tables based on the operation acting on an address.
   *
   * @param address The address being operated on.
   * @param op The operation (i.e., read or write)
   */
  void update(uint64_t address, operation op);

  /**
   * @return The total number of requests modeled.
   */
  std::uint64_t count() const
  {
    return read_count + write_count;
  }

  /// The stack distance profile.
  sdc_table sdc;
  /// The stride pattern profile.
  spc_table spc;

  /// The number of times the SDC table was updated.
  std::uint64_t sdc_update_count = 0;
  /// The number of reads profiled.
  std::uint64_t read_count = 0;
  /// The number of writes profiled.
  std::uint64_t write_count = 0;

  std::uint64_t min_address;
  std::uint64_t max_address = 0;
};
}

#endif //STM_CLONING_PROFILE_HPP
