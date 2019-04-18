#include "reuse-distance/olken-tree.hpp"

#include <cassert>
#include <reuse-distance/olken-tree.hpp>

namespace reuse_distance {

olken_tree::olken_tree() : m_nil(std::make_unique<node>(-1, -1)), m_root(m_nil.get())
{
  m_nil->left = m_nil.get();
  m_nil->right = m_nil.get();
  m_nil->parent = m_nil.get();
}

bool olken_tree::empty() const
{
  return m_hashmap.empty();
}

std::size_t olken_tree::size() const
{
  return m_hashmap.size();
}

olken_tree::node *olken_tree::successor(olken_tree::node *x) const
{
  node *y = x->right;
  if(y != m_nil.get()) {
    // The node x has a right subtree, the successor should be here.
    while(y->left != m_nil.get()) {
      y = y->left;
    }

    return y;
  }

  // The node x does not have a right subtree, check the parents.
  y = x->parent;
  while(y != m_nil.get() && x == y->right) {
    x = y;
    y = y->parent;
  }

  return y;
}

olken_tree::node *olken_tree::predecessor(olken_tree::node *x) const
{
  node *y = x->left;
  if(y != m_nil.get()) {
    // The node x has a left subtree, the predecessor should be here.
    while(y->right != m_nil.get()) {
      y = y->right;
    }

    return y;
  }

  // The node x does not have a left subtree, check the parents.
  y = x->parent;
  while(y != m_nil.get() && x == y->left) {
    x = y;
    y = y->parent;
  }

  return y;
}

olken_tree::node *olken_tree::most_recently_used() const
{
  node *x = m_root;
  while(x->right != m_nil.get()) {
    x = x->right;
  }

  return x;
}

olken_tree::node *olken_tree::least_recently_used() const
{
  node *x = m_root;
  while(x->left != m_nil.get()) {
    x = x->left;
  }

  return x;
}

double olken_tree::calculate_position(olken_tree::node const *n) const
{
  assert(m_root->parent == m_nil.get());

  double position = n->right->size;

  node *i = n->parent;
  while(i != m_nil.get()) {
    if(n->time < i->time) {
      // n is in the left subtree of i
      position += i->right->size;
      position += 1; // for the node i
    }

    i = i->parent; // Traverse up to the root node
  }

  return position;
}

olken_tree::node *olken_tree::find_address(std::uint64_t address) const
{
  auto const it = m_hashmap.find(address);

  if(it == m_hashmap.end()) {
    return nullptr;
  }

  return it->second;
}

olken_tree::node *olken_tree::insert(std::uint64_t const time, std::uint64_t const address)
{
  auto new_node = new node(time, address);
  new_node->size = 1;
  m_hashmap[address] = new_node;

  node *z = new_node;
  node *y = m_nil.get();
  node *x = m_root;

  while(x != m_nil.get()) {
    x->size++;
    y = x;

    if(x->time > z->time) {
      x = x->left;
    } else {
      x = x->right;
    }
  }

  z->parent = y;
  if(y == m_nil.get()) {
    m_root = z;
  } else if(z->time < y->time) {
    y->left = z;
  } else {
    y->right = z;
  }

  z->left = m_nil.get();
  z->right = m_nil.get();
  z->red = true;

  fix_insert(z);

  return new_node;
}

void olken_tree::erase(node *z)
{
  assert(m_root->parent == m_nil.get());

  auto const address = z->address;

  // y will either be z or z's successor, which has no left child
  node *y = nullptr;
  if(z->left == m_nil.get() || z->right == m_nil.get()) {
    // z has at most one child
    y = z;
  } else {
    // z has two children
    y = successor(z);
  }

  // y will be spliced out, update subtree sizes by traversing back to root
  node *i = y->parent;
  do {
    if(i->size > 0) {
      i->size--;
    }

    i = i->parent;
  } while(i != m_nil.get());

  // x will either be nil or the non-nil child of y
  node *x = nullptr;
  if(y->left != m_nil.get()) {
    x = y->left;
  } else {
    x = y->right;
  }

  x->parent = y->parent;

  if(y->parent == m_nil.get()) {
    m_root = x;
  } else {
    if(y == y->parent->left) {
      y->parent->left = x;
    } else {
      y->parent->right = x;
    }
  }

  if(y != z) {
    z->time = y->time;
    z->address = y->address;
    m_hashmap[z->address] = z;
  }

  if(!y->red) {
    fix_delete(x);
  }

  delete y;
  m_hashmap.erase(address);
}

void olken_tree::fix_insert(olken_tree::node *z)
{
  while(z->parent->red) {
    if(z->parent == z->parent->parent->left) {
      // z's parent is on the left side of x's grandparent

      node *y = z->parent->parent->right;
      if(y->red) {
        z->parent->red = false;
        y->red = false;
        z->parent->parent->red = true;

        z = z->parent->parent;
      } else {
        if(z == z->parent->right) {
          // z is on the right side of its parent
          z = z->parent;
          rotate_left(z);
        }

        z->parent->red = false;
        z->parent->parent->red = true;
        rotate_right(z->parent->parent);
      }
    } else {
      // symmetric to the above if-clause

      node *y = z->parent->parent->left;
      if(y->red) {
        z->parent->red = false;
        y->red = false;
        z->parent->parent->red = true;

        z = z->parent->parent;
      } else {
        if(z == z->parent->left) {
          z = z->parent;
          rotate_right(z);
        }

        z->parent->red = false;
        z->parent->parent->red = true;
        rotate_left(z->parent->parent);
      }
    }
  }

  m_root->red = false;
}

void olken_tree::fix_delete(olken_tree::node *x)
{
  while(x != m_root && !x->red) {
    if(x == x->parent->left) {
      node *w = x->parent->right;

      if(w->red) {
        // Case 1
        w->red = false;
        x->parent->red = true;

        rotate_left(x->parent);
        w = x->parent->right;
      }

      if(!w->left->red && !w->right->red) {
        // Case 2
        w->red = true;
        x = x->parent;
      } else {
        if(!w->right->red) {
          // Case 3
          w->left->red = false;
          w->red = true;

          rotate_right(w);
          w = x->parent->right;
        }

        // Case 4
        w->red = x->parent->red;
        x->parent->red = false;
        w->right->red = false;

        rotate_left(x->parent);
        x = m_root;
      }
    } else {
      node *w = x->parent->left;

      if(w->red) {
        // Case 1
        w->red = false;
        x->parent->red = true;

        rotate_right(x->parent);
        w = x->parent->left;
      }

      if(!w->right->red && !w->left->red) {
        // Case 2
        w->red = true;
        x = x->parent;
      } else {
        if(!w->left->red) {
          // Case 3
          w->right->red = false;
          w->red = true;

          rotate_left(w);
          w = x->parent->left;
        }

        // Case 4
        w->red = x->parent->red;
        x->parent->red = false;
        w->left->red = false;

        rotate_right(x->parent);
        x = m_root;
      }
    }
  }

  x->red = false;
}

void olken_tree::rotate_left(olken_tree::node *x)
{
  // y is x's right subtree
  node *y = x->right;
  // Turn y's left subtree into x's right subtree
  x->right = y->left;

  if(y->left != m_nil.get()) {
    y->left->parent = x;
  }

  // Link x's parent to y
  y->parent = x->parent;

  if(x->parent == m_nil.get()) {
    m_root = y;
  } else {
    if(x == x->parent->left) {
      x->parent->left = y;
    } else {
      x->parent->right = y;
    }
  }

  // Put x on y's left
  y->left = x;
  x->parent = y;

  y->size = x->size;
  x->size = x->left->size + x->right->size + 1;
}

void olken_tree::rotate_right(olken_tree::node *y)
{
  node *x = y->left;
  y->left = x->right;

  if(x->right != m_nil.get()) {
    x->right->parent = y;
  }

  x->parent = y->parent;

  if(y->parent == m_nil.get()) {
    m_root = x;
  } else {
    if(y == y->parent->left) {
      y->parent->left = x;
    } else {
      y->parent->right = x;
    }
  }

  x->right = y;
  y->parent = x;

  x->size = y->size;
  y->size = y->left->size + y->right->size + 1;
}
} // namespace reuse_distance