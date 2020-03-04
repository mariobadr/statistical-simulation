#ifndef MOCKTAILS_PROFILE_HPP
#define MOCKTAILS_PROFILE_HPP

#include <hrd/profile.hpp>
#include <stm/profile.hpp>

#include "mocktails/hierarchy.hpp"
#include "mocktails/model.hpp"

namespace mocktails {

enum class model_type : int {
  mocktails = 0, stm = 1, hrd = 2
};

/**
 * The statistical profile.
 */
class profile {
public:
  /**
   * Constructor.
   *
   * Creates an empty profile.
   */
  profile(std::uint32_t identifier, model_type t);

  /**
   * Constructor.
   *
   * Creates a model for each leaf node of the hierarchy.
   */
  profile(std::uint32_t identifier, model_type t, hierarchy const &h);

  std::uint32_t id;
  model_type type;
  std::uint64_t model_count = 0;

  // Models the requests of leaf nodes in the hierarchy.
  std::map<std::uint32_t, model<simple_model>> simple_leaves;
  std::map<std::uint32_t, model<stm::profile>> stm_leaves;
  std::map<std::uint32_t, model<hrd::profile>> hrd_leaves;

private:
  void create_hierarchy(hierarchy const &h, std::uint32_t node_id, std::size_t level_id);
};
} // namespace mocktails

#endif //MOCKTAILS_PROFILE_HPP
