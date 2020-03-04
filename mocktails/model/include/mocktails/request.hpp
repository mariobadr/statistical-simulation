#ifndef MOCKTAILS_REQUEST_HPP
#define MOCKTAILS_REQUEST_HPP

#include <cstdint>
#include <iosfwd>

#include "mocktails/address.hpp"

namespace mocktails {

/**
 * The different types of requests.
 */
enum class operation : int { read = 0, write = 1 };

std::ostream &operator<<(std::ostream &stream, operation const &op);

/**
 * A memory request.
 */
struct request {
  /**
   * Constructor.
   */
  request(std::uint64_t time, operation o, address_t a, std::uint64_t s)
      : timestamp(time), op(o), address(a), size(s)
  {
  }

  /**
   * The time the request occured.
   */
  std::uint64_t timestamp;

  /**
   * The type of request.
   */
  operation op;

  /**
   * The data address.
   */
  address_t address;

  /**
   * The number of bytes requested.
   */
  std::uint64_t size;
};

/**
 * Output a request to a stream.
 */
std::ostream &operator<<(std::ostream &stream, request const &r);

/**
 * Compare requests based on their timestamp.
 *
 * @return true if lhs comes before rhs in time, false otherwise.
 */
bool operator<(request const &lhs, request const &rhs);

} // namespace mocktails

#endif //MOCKTAILS_REQUEST_HPP
