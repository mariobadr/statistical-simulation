#ifndef MOCKTAILS_SYNTHESIS_HPP
#define MOCKTAILS_SYNTHESIS_HPP

#include <queue>
#include <random>

#include <mocktails/profile.hpp>

namespace mocktails {
/**
 * Generates synthetic requests based on an mocktails::profile.
 */
class synthesiser {
public:
  /**
   * Constructor.
   *
   * @param p The distributions to use to generate synthetic requests.
   */
  explicit synthesiser(profile &p);

  /**
   * @return true if the synthesiser has more requests to generate, false otherwise.
   */
  bool has_more_requests() const;

  /**
   * Synthesize a request.
   */
  request generate_next_request();

private:
  std::mt19937 m_rng;

  std::priority_queue<request> requests;
};
} // namespace mocktails

#endif //MOCKTAILS_SYNTHESIS_HPP
