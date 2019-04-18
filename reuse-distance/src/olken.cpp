#include <reuse-distance/olken.hpp>

#include "reuse-distance/olken.hpp"

#include <cassert>

namespace reuse_distance {

double compute_distance(olken_tree const &tree, uint64_t address)
{
  auto node = tree.find_address(address); // O(1)

  if(node == nullptr) {
    return std::numeric_limits<double>::infinity();
  }

  return tree.calculate_position(node);
}

void update(olken_tree &tree, std::uint64_t address, std::uint64_t time)
{
  auto node = tree.find_address(address); // O(1)

  if(node != nullptr) {
    tree.erase(node);
  }

  tree.insert(time, address);
}

} // namespace reuse_distance