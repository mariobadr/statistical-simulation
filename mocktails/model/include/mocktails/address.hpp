#ifndef MOCKTAILS_ADDRESS_HPP
#define MOCKTAILS_ADDRESS_HPP

#include <cstdint>
#include <limits>

namespace mocktails {

/**
 * Use unsigned integers to represent addresses.
 */
using address_t = std::uint64_t;

/**
 * Use integers to represent strides.
 *
 * Strides can be positive (next address) or negative (previous address).
 */
using stride_t = std::int32_t;

/**
 * Calculate the stride between two addresses.
 */
inline stride_t calculate_stride(address_t begin, address_t end)
{
  return static_cast<stride_t>(begin - end);
}

/**
 * A range of addresses.
 */
struct address_range {
  /**
   * The start of the address range.
   */
  address_t start = std::numeric_limits<address_t>::max();

  /**
   * The end of the address range.
   */
  address_t end = std::numeric_limits<address_t>::min();

  /**
   * Number of requests that belong to this address range.
   */
  std::size_t count = 0;

  /**
   * Check if the address is within this range.
   *
   * @param address The address to test.
   * @return true if the address is within the range, false otherwise.
   */
  bool contains(address_t address) const;

  /**
   * Check if another address range intersects with this one.
   *
   * @param other The other address range.
   * @return true if the ranges intersect, false otherwise.
   */
  bool intersects(address_range const &other) const;

  /**
   * Expand this address range to encompass another.
   *
   * @param extra_range The range to encompass.
   */
  void expand(address_range const &extra_range);
};
}
#endif //MOCKTAILS_ADDRESS_HPP
