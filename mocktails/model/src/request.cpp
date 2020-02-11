#include "mocktails/request.hpp"

#include <ostream>
#include <mocktails/request.hpp>

namespace mocktails {

std::ostream &operator<<(std::ostream &stream, operation const &op)
{
  switch(op) {
  case operation::read:
    stream << "R";
    break;
  case operation::write:
    stream << "W";
    break;
  }

  return stream;
}

std::ostream &operator<<(std::ostream &stream, request const &r)
{
  stream << r.timestamp << ",";
  stream << r.op << ",";
  stream << r.address << ",";
  stream << r.size;

  return stream;
}

bool operator<(request const &lhs, request const &rhs)
{
  return rhs.timestamp < lhs.timestamp;
}
} // namespace mocktails