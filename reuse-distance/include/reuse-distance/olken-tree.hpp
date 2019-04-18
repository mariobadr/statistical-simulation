#ifndef REUSE_DISTANCE_OLKEN_TREE_HPP
#define REUSE_DISTANCE_OLKEN_TREE_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace reuse_distance {

/**
 * A tree structure for reuse-distance tracking.
 *
 * The data structure follows the description by Olken in, "Efficient methods for calculating the success function of
 * fixed space replacement policies."
 *
 * There are actually two data structures: an ordered statistics tree and a hashmap.
 *
 * The tree is key'd according to the logical time (i.e., order) of memory accesses. The hashmap is indexed based on
 * the address of the memory access. In this way, the hashmap can find a node from the tree in O(1) , and the tree can
 * be used to compute the reuse distance in O(log n).
 */
class olken_tree {
public:
  /**
   * A node in the tree.
   *
   * Contains a payload of the time last accessed and the memory address.
   *
   * In addition, metadata for maintaining a balanced tree via the red-black tree approach from CLRS (Chapter 13).
   * In addition, metadata for maintaining the "weight" of each node (CLRS, Chapter 14).
   */
  struct node {
    /**
     * Constructor.
     *
     * @param t Time of last access.
     * @param a Address accessed.
     */
    explicit node(std::uint64_t t, std::uint64_t a) : time(t), address(a)
    {
    }

    /**
     * The time last accessed.
     */
    std::uint64_t time = 0;

    /**
     * The address of the memory access;
     */
    std::uint64_t address = 0;

    /**
     * The weight of the node's subtree.
     *
     * Should be 1 for leaf nodes.
     */
    int size = 0;

    /**
     * True if red, false if black.
     */
    bool red = false;

    /**
     * The left subtree.
     */
    node *left = nullptr;

    /**
     * The right subtree.
     */
    node *right = nullptr;

    /**
     * The node's parent.
     */
    node *parent = nullptr;
  };

  /**
   * Constructor.
   *
   * Sets up the nil sentinel.
   */
  olken_tree();

  /**
   * Check if the tree is empty.
   *
   * @return true if the tree is empty, false otherwise.
   */
  bool empty() const;

  /**
   * @return The number of nodes in the tree.
   */
  std::size_t size() const;

  /**
   * Find the node with the next timestamp.
   *
   * @param x The node with the starting timestamp.
   *
   * @return The node with the next timestamp.
   */
  node *successor(node *x) const;

  /**
   * Find the node with the previous timestamp.
   *
   * @param x The node with the starting timestamp.
   *
   * @return The node with the previous timestamp.
   */
  node *predecessor(node *x) const;

  /**
   * @return The node with the greatest timestamp.
   */
  node *most_recently_used() const;

  /**
   * @return The node with the smallest timestamp.
   */
  node *least_recently_used() const;

  /**
   * Calculate the stack position.
   *
   * Traverses from the passed in node to the root, accumulating weights along the way.
   *
   * @param n The node to begin the traversal at.
   *
   * @return The accumulated weights, analogous to the stack position of the memory address.
   */
  double calculate_position(node const *n) const;

  /**
   * Find the node based on the memory address accessed.
   *
   * @param address The memory address to search for.
   *
   * @return The node that accessed that address.
   */
  node *find_address(std::uint64_t address) const;

  /**
   * Add a node to the tree and hashmap.
   *
   * @param time The time the node was accessed.
   * @param address The address of the memory access.
   *
   * @return The inserted node.
   */
  olken_tree::node *insert(std::uint64_t time, std::uint64_t address);

  /**
   * Delete a node from the tree and the hashmap.
   *
   * @param z The node to be erased.
   */
  void erase(node *z);

private:
  std::unique_ptr<node> m_nil;
  node *m_root;

  std::unordered_map<std::uint64_t, olken_tree::node *> m_hashmap;

  void fix_insert(node *z);
  void fix_delete(node *x);

  void rotate_left(node *x);
  void rotate_right(node *y);
};
} // namespace reuse_distance

#endif //REUSE_DISTANCE_OLKEN_TREE_HPP
