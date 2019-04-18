#ifndef STM_CLONING_SYNTHESIS_HPP
#define STM_CLONING_SYNTHESIS_HPP

#include <random>

#include "stm/profile.hpp"

namespace stm {

/**
 * Generates synthetic requests based on an stm::profile.
 */
class synthesiser {
public:
  /**
   * Constructor.
   *
   * @param p The distributions to use to generate synthetic requests.
   */
  explicit synthesiser(profile p);

  /**
   * Synthesize a request.
   *
   * @param address The synthesized address.
   * @param op The synthesized operation.
   */
  void generate_next_request(std::uint64_t &address, operation &op);

private:
  std::mt19937 m_rng;

  profile m_profile;

  history_sequence m_history;

  bool m_first_request = true;
};
} // namespace stm

#endif //STM_CLONING_SYNTHESIS_HPP
