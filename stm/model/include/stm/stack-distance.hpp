#ifndef STM_CLONING_SDC_TABLE_HPP
#define STM_CLONING_SDC_TABLE_HPP

#include <cstdint>
#include <vector>

#include "reuse-distance/olken.hpp"

namespace stm {

/**
 * The Stack Distance Count (SDC) table captures tight reuse distances in a tagged cache-like structure.
 */
class sdc_table {
public:
  /**
   * Constructor.
   *
   * @param num_rows Number of rows in the table.
   * @param num_columns Number of reuse distances to track, starting from 0.
   */
  sdc_table(std::size_t num_rows, std::size_t num_columns);

  /**
   * Copy Constructor.
   *
   * Does not copy intermediate data structures used for tracking.
   */
  sdc_table(sdc_table const &table);

  /**
   * Copy on assignment.
   *
   * Resets intermediate data structures used for tracking.
   */
  sdc_table& operator=(sdc_table const &table);

  /**
   * Update the table based on the accessed address.
   *
   * If there is a tag match, the count is updated.
   * If there is no tag match, and the entry is empty, the count is updated.
   * If there is no tag match, and the entry is not empty, the older tag is replaced.
   *
   * @param address The address that is being accessed.
   *
   * @return true if there was a tag match.
   */
  bool update(uint64_t address);

  std::size_t row_size() const
  {
    return row_count;
  }

  std::size_t column_size() const
  {
    return col_count;
  }

  struct column {
    /// The tag of the address.
    std::uint64_t tag = 0;
    /// All tags start off invalid.
    bool valid = false;
    /// The count of this column (stack distance).
    std::uint64_t count = 0;
  };

  /**
   * A row in the SDC Table.
   */
  struct row {
    std::vector<column> columns;
  };

  /// All the rows in the SDC table.
  std::vector<row> rows;

private:
  std::size_t row_count;

  std::size_t col_count;

  reuse_distance::olken_tree olken;
  std::uint64_t time = 0;
};
} // namespace stm

#endif //STM_CLONING_SDC_TABLE_HPP
