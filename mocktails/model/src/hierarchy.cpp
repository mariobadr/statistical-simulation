
#include <mocktails/hierarchy.hpp>

#include "mocktails/hierarchy.hpp"

namespace mocktails {

hierarchy::hierarchy(configuration config, partition root_partition) : m_config(std::move(config))
{
  m_nodes.emplace(m_root_id, hierarchy::node(std::move(root_partition)));
}

std::uint32_t hierarchy::add_partition(std::uint32_t parent_node, partition p)
{
  auto const child_id = m_last_id++;

  auto parent_it = m_nodes.find(parent_node);
  if(parent_it == m_nodes.end()) {
    throw std::runtime_error("Could not find parent node " + std::to_string(parent_node));
  } else {
    parent_it->second.children.emplace_back(child_id);
  }

  m_parents.emplace(child_id, parent_it->first);
  m_nodes.emplace(child_id, node(std::move(p)));

  return child_id;
}

partition &hierarchy::get_partition(std::uint32_t node_id)
{
  auto it = m_nodes.find(node_id);
  if(it == m_nodes.end()) {
    throw std::runtime_error("Could not find node " + std::to_string(node_id));
  }

  return it->second.p;
}

partition const &hierarchy::get_partition(std::uint32_t node_id) const
{
  auto it = m_nodes.find(node_id);
  if(it == m_nodes.end()) {
    throw std::runtime_error("Could not find node " + std::to_string(node_id));
  }

  return it->second.p;
}

std::vector<std::uint32_t> const &hierarchy::get_children(std::uint32_t node_id) const
{
  auto node_it = m_nodes.find(node_id);

  if(node_it == m_nodes.end()) {
    throw std::runtime_error("Could not find node " + std::to_string(node_id));
  }

  return node_it->second.children;
}
}