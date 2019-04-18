#ifndef REUSE_DISTANCE_OLKEN_HPP
#define REUSE_DISTANCE_OLKEN_HPP

#include <limits>

#include <reuse-distance/olken-tree.hpp>

/**
 * Calculate reuse distance.
 */
namespace reuse_distance {

/**
 * Compute the stack distance for the given address.
 *
 * Complexity: O(log n)
 *
 * @param tree The tree to search.
 * @param address The location of the time last accessed.
 *
 * @return The number of nodes that were referenced between the time last accessed and now.
 */
double compute_distance(olken_tree const &tree, uint64_t address);

/**
 * Update the time last accessed and the mapping of the address.
 *
 * Complexity: O(1) amoritized
 *
 * @param tree The tree to search.
 * @param address The address that has become the most recent reference.
 * @param time The time of the access.
 */
void update(olken_tree &tree, std::uint64_t address, std::uint64_t time);

} // namespace reuse_distance

#endif //REUSE_DISTANCE_OLKEN_HPP
