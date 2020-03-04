#ifndef MOCKTAILS_HIERARCHY_HPP
#define MOCKTAILS_HIERARCHY_HPP

#include <map>
#include <vector>

#include "mocktails/partition.hpp"

namespace mocktails {

class hierarchy {
public:
  /**
   * The configuration of a hierarchy.
   */
  struct configuration {
    std::vector<partition::configuration> levels;
  };

public:
  /**
   * Constructor.
   */
  hierarchy(configuration config, partition root_partition);

  /**
   * Add a partition to a parent node in the hierarchy.
   *
   * @return The node ID of the newly added partition.
   */
  std::uint32_t add_partition(std::uint32_t parent_node, partition p);

  /**
   * Get a reference to the partition at the specified node.
   */
  partition &get_partition(std::uint32_t node_id);

  /**
   * Get a const reference to the partition at the specified node.
   */
  partition const &get_partition(std::uint32_t node_id) const;

  /**
   * Get the children of a specific node.
   */
  std::vector<std::uint32_t> const &get_children(std::uint32_t node_id) const;

  /**
   * Identifier of the root node.
   */
  std::uint32_t root_id() const
  {
    return m_root_id;
  }

private:
  /**
   * A node in the hierarchy.
   */
  struct node {
    /**
     * Constructor.
     */
    node() = default;

    /**
     * Constructor.
     */
    explicit node(partition new_partition) : p(std::move(new_partition))
    {
    }

    // The partition belonging to this node.
    partition p;

    // The IDs of nodes that are this node's children.
    std::vector<std::uint32_t> children;
  };

private:
  std::uint32_t m_root_id = 0;
  std::uint32_t m_last_id = 1;

  // Configuration of each level of the hierarchy.
  configuration m_config;
  // A mapping of node ID to nodes in the hierarchy.
  std::map<std::uint32_t, node> m_nodes;
  // A mapping of a child's node ID to its parent's node ID.
  std::map<std::uint32_t, std::uint32_t> m_parents;
};
}

#endif //MOCKTAILS_HIERARCHY_HPP
