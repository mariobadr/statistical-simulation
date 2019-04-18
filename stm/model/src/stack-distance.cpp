#include "stm/stack-distance.hpp"

#include <cmath>

namespace stm {

sdc_table::sdc_table(std::size_t num_rows, std::size_t num_columns)
    : rows(num_rows), row_count(num_rows), col_count(num_columns)
{
  for(auto &r : rows) {
    r.columns.resize(num_columns);
  }
}

sdc_table::sdc_table(sdc_table const &table)
    : rows(table.rows), row_count(table.row_count), col_count(table.col_count)
{
}

sdc_table &sdc_table::operator=(sdc_table const &table)
{
  rows = table.rows;
  row_count = table.row_count;
  col_count = table.col_count;

  olken = reuse_distance::olken_tree{};
  time = 0;

  return *this;
}


bool sdc_table::update(uint64_t const address)
{
  if(rows.empty()) {
    return false;
  }

  // assume that rows.size() is a power of 2
  auto const index_bits = static_cast<std::uint32_t>(std::log2(rows.size()));
  // most significant bits are the tag
  auto const tag = address >> index_bits;
  // least significant bits are used as the index
  auto const row_index = address & ((1u << index_bits) - 1u);

  auto &r = rows.at(row_index);

  // the stack distance needs to be clamped to the maximum column index
  auto const stack_distance =
      static_cast<std::size_t>(reuse_distance::compute_distance(olken, address));
  reuse_distance::update(olken, address, time);
  time = time + 1;

  auto const column_index = std::min(stack_distance, column_size() - 1);

  auto &c = r.columns.at(column_index);

  if(!c.valid) {
    // first time accessing this row
    c.valid = true;
    c.tag = tag;
    c.count++;

    return true;
  } else if(c.tag != tag) {
    // tag miss, keep most recently used tag
    c.tag = tag;

    return false;
  } else {
    // tag hit
    c.count++;

    return true;
  }
}

} // namespace stm