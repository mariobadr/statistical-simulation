#include "mocktails/address.hpp"

#include <algorithm>
#include <mocktails/address.hpp>

namespace mocktails {

bool address_range::contains(address_t const address) const
{
  return address >= start && address <= end;
}

bool address_range::intersects(address_range const &other) const
{
  return start <= other.end && other.start <= end;
}

void address_range::expand(address_range const &extra_range)
{
  start = std::min(start, extra_range.start);
  end = std::max(end, extra_range.end);
}
} // namespace mocktails