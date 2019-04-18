#ifndef HRD_CLONING_REQUEST_TYPE_HPP
#define HRD_CLONING_REQUEST_TYPE_HPP

#include <array>

namespace hrd {

/**
 * The type of operation.
 */
enum class operation : std::size_t { read = 0, write = 1 };

/**
 * Number of operations supported.
 */
constexpr std::size_t OPERATION_COUNT = 2;

/**
 * The frequency of reads and writes.
 */
using operation_histogram = std::array<std::uint64_t, OPERATION_COUNT>;

/**
 * Model a memory location as being in one of three possible states.
 */
enum class memory_state : std::size_t {
  /** The memory location has never been touched. */
  invalid = 0,
  /** The memory location has been read, but never written to. */
  clean = 1,
  /** The memory location has been written to. */
  dirty = 2
};


/**
 * Number of memory states supported.
 */
constexpr std::size_t MEMORY_STATE_COUNT = 3;

} // namespace hrd

#endif //HRD_CLONING_REQUEST_TYPE_HPP
