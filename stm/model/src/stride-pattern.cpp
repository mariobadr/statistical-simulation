#include "stm/stride-pattern.hpp"

#include <cassert>

#include "hash.hpp"

namespace stm {

inline std::int64_t calculate_stride(std::uint64_t begin, std::uint64_t end)
{
  return static_cast<std::int64_t>(begin - end);
}

history_sequence::history_sequence(size_t depth) : sequence(depth)
{
}

void history_sequence::add(std::int64_t observation)
{
  sequence.push_front(observation);
  sequence.pop_back();
}

size_t history_sequence::size() const noexcept
{
  return sequence.size();
}

history_sequence::Container::const_iterator history_sequence::begin() const noexcept
{
  return sequence.begin();
}

history_sequence::Container::const_iterator history_sequence::end() const noexcept
{
  return sequence.end();
}

history_sequence::Container::value_type const &history_sequence::back() const
{
  return sequence.back();
}

std::int64_t history_sequence::distance(history_sequence const &hs)
{
  assert(size() == hs.size());

  int distance = 0;
  for(std::size_t i = 0; i < size(); ++i) {
    distance += (sequence[i] != hs.sequence[i]);
  }

  return distance;
}

bool history_table::empty() const
{
  return rows.empty();
}

size_t history_table::size() const
{
  return rows.size();
}

void history_table::increment(std::uint64_t index,
    std::int64_t observation,
    history_sequence const &pattern)
{
  auto it = rows.find(index);
  if(it == rows.end()) {
    // The row did not exist, so we create it at the index with
    // the corresponding stride pattern.
    std::tie(it, std::ignore) = rows.emplace(index, pattern);
  }

  it->second.counts[observation]++;
}

void history_table::set(history_sequence pattern, std::int64_t observation, std::uint64_t count)
{
  auto const index = hash_unique(pattern);

  auto it = rows.find(index);
  if(it == rows.end()) {
    // The row did not exist, so we create it at the index with
    // the corresponding stride pattern.
    std::tie(it, std::ignore) = rows.emplace(index, std::move(pattern));
  }

  it->second.counts[observation] = count;
}

history_table::Container::const_iterator history_table::begin() const noexcept
{
  return rows.begin();
}

history_table::Container::const_iterator history_table::end() const noexcept
{
  return rows.end();
}

spc_table::spc_table(std::size_t stride_depth) : last_M_strides(stride_depth)
{
}

void spc_table::update(std::uint64_t address)
{
  if(first_request) {
    first_request = false;

    last_address = start_address = address;

    return;
  }

  auto const stride = calculate_stride(address, last_address);
  last_address = address;

  // "The history of the last M stride value is ... given to a hash function to index the SP tables..."
  auto const index = hash_unique(last_M_strides);

  // "... the new stride value is calculated by subtracting the address of the new access with the last access."
  // "... the appropriate entry is updated..."
  stride_patterns.increment(index, stride, last_M_strides);

  // update for next address access
  last_M_strides.add(stride);
}
} // namespace stm