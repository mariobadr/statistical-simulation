#include "stm/synthesis.hpp"

#include <cassert>

#include "hash.hpp"

namespace stm {

template <typename Type>
void converge(Type &value)
{
  assert(value > 0);
  auto new_value = value - 1;

  if(new_value > value) {
    value = 0;
  } else {
    value = new_value;
  }
}

bool has_converged(history_table::row const &row)
{
  for(auto const &pair : row.counts) {
    if(pair.second > 0) {
      return false;
    }
  }

  return true;
}

operation generate_operation(std::mt19937 &rng, std::uint64_t &reads, std::uint64_t &writes)
{
  auto const read_count = static_cast<double>(reads);
  auto const write_count = static_cast<double>(writes);

  std::discrete_distribution<> distribution({read_count, write_count});
  auto const index = distribution(rng);

  if(index == 0) {
    converge(reads);

    return operation::read;
  }

  converge(writes);
  return operation::write;
}

std::uint64_t generate_sdc_address(std::mt19937 &rng, sdc_table &table)
{
  // Select a row from the SDC table.
  std::vector<std::uint64_t> row_counts(table.rows.size());

  std::size_t i = 0;
  for(auto const &row : table.rows) {
    for(auto const &col : row.columns) {
      row_counts[i] += col.count;
    }

    ++i;
  }

  std::discrete_distribution<std::uint64_t> row_distribution(row_counts.begin(), row_counts.end());
  auto const row_index = row_distribution(rng);

  // Select a column from the SDC row.
  std::vector<std::uint64_t> col_counts;
  for(auto const &col : table.rows[row_index].columns) {
    col_counts.push_back(col.count);
  }

  std::discrete_distribution<std::uint64_t> col_distribution(col_counts.begin(), col_counts.end());
  auto const col_index = col_distribution(rng);

  // Generate an address based on the table's cell.
  auto &cell = table.rows[row_index].columns[col_index];
  auto const index_bits = static_cast<unsigned>(std::log2(table.rows.size()));
  std::uint64_t const address = (cell.tag << index_bits) | row_index;

  converge(cell.count);

  return address;
}

std::int64_t generate_spc_stride(std::mt19937 &rng, history_table &table, history_sequence &history)
{
  auto const index = hash_unique(history);

  auto it = table.rows.find(index);

  if(it == table.rows.end()) {
    auto min_distance = std::numeric_limits<std::int64_t>::max();

    for(auto itt = table.rows.begin(); itt != table.rows.end(); ++itt) {
      auto const distance = history.distance(itt->second.pattern);

      if(distance < min_distance) {
        it = itt;
        min_distance = distance;
      }
    }

    assert(it != table.rows.end());
    history = it->second.pattern;
  }

  std::int64_t stride;
  if(it->second.counts.size() == 1) {
    stride = it->second.counts.begin()->first;
  } else {
    std::vector<std::int64_t> strides;
    std::vector<std::uint64_t> counts;
    for(auto const &col : it->second.counts) {
      strides.push_back(col.first);
      counts.push_back(col.second);
    }

    std::discrete_distribution<std::uint64_t> distribution(counts.begin(), counts.end());
    auto const col_index = distribution(rng);
    stride = strides[col_index];
  }

  history.add(stride);
  converge(it->second.counts[stride]);

  if(has_converged(it->second)) {
    table.rows.erase(it);
  }

  return stride;
}

std::uint64_t generate_address(std::mt19937 &rng, profile &p, history_sequence &history)
{
  auto const sdc_count = static_cast<double>(p.sdc_update_count);
  auto const spc_count = static_cast<double>(p.count() - p.sdc_update_count);

  std::discrete_distribution<> distribution({sdc_count, spc_count});
  auto const index = distribution(rng);

  if(index == 0) {
    converge(p.sdc_update_count);
    return generate_sdc_address(rng, p.sdc);
  } else {
    if(p.spc.first_request) {
      p.spc.first_request = false;

      return p.spc.start_address;
    }

    auto const stride = generate_spc_stride(rng, p.spc.stride_patterns, history);
    std::uint64_t const address = p.spc.last_address + stride;
    p.spc.last_address = address;

    return address;
  }
}

std::uint64_t keep_in_range(std::uint64_t address, std::uint64_t min, std::uint64_t max)
{
  if(address > max || address < min) {
    const auto size = max - min;
    address = ((address - size) % size) + min;
  }

  return address;
}

synthesiser::synthesiser(profile p)
    : m_profile(std::move(p)), m_history(m_profile.spc.stride_pattern_depth())
{
}

void synthesiser::generate_next_request(std::uint64_t &address, operation &op)
{
  if(m_profile.sdc.row_size() > 0 && m_first_request) {
    m_first_request = false;
    converge(m_profile.sdc_update_count);
    address = generate_sdc_address(m_rng, m_profile.sdc);
  } else {
    address = generate_address(m_rng, m_profile, m_history);
  }

  address = keep_in_range(address, m_profile.min_address, m_profile.max_address);
  op = generate_operation(m_rng, m_profile.read_count, m_profile.write_count);
}
} // namespace stm