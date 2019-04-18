#ifndef STM_CLONING_HASH_HPP
#define STM_CLONING_HASH_HPP

#include <functional>

namespace stm {

template <typename Container>
std::uint64_t hash_unique(Container const &c)
{
  // based on the boost hash_combine functions
  // regarding the magic number, see: https://stackoverflow.com/a/4948967
  static constexpr auto MAGIC = 0x9e3779b9;

  // use std::hash to return unsigned numbers
  std::hash<typename Container::value_type> hasher;
  std::uint64_t result = c.size();

  for(const auto &i : c) {
    result ^= hasher(i) + MAGIC + (result << 6u) + (result >> 2u);
  }

  return result;
}
}

#endif //STM_CLONING_HASH_HPP
