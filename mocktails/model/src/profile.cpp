#include "mocktails/profile.hpp"

namespace mocktails {

profile::profile(std::uint32_t identifier, model_type t) : id(identifier), type(t)
{
}

profile::profile(std::uint32_t identifier, model_type t, hierarchy const &h)
    : profile(identifier, t)
{
  create_hierarchy(h, h.root_id(), 0u);
}

void profile::create_hierarchy(hierarchy const &h, std::uint32_t node_id, std::size_t level_id)
{
  auto const &children = h.get_children(node_id);
  if(children.empty()) {
    // This node has no children, therefore it is a leaf node.
    auto const &partition = h.get_partition(node_id);

    // Generate a simple model for this partition.
    if(type == model_type::mocktails) {
      simple_leaves.emplace(node_id, create_simple_model(partition.requests));
    } else if(type == model_type::stm) {
      stm_leaves.emplace(node_id, create_stm_model(partition.requests));
    } else if(type == model_type::hrd) {
      hrd_leaves.emplace(node_id, create_hrd_model(partition.requests));
    }

    model_count++;
    return;
  }

  // Recursively create the hierarchy for each child who is also a parent.
  for(auto const &child : children) {
    create_hierarchy(h, child, level_id + 1);
  }
}
} // namespace mocktails