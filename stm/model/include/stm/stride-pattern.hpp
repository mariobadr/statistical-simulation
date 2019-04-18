#ifndef STM_CLONING_SPC_TABLE_HPP
#define STM_CLONING_SPC_TABLE_HPP

#include <cstdint>
#include <deque>
#include <map>
#include <unordered_map>

namespace stm {

/**
 * Maintains a fixed-size history of observations.
 */
class history_sequence {
private:
  /**
   * We use a deque because we want push/pop functionality with fast
   * insertion/deletion
   */
  using Container = std::deque<std::int64_t>;

  Container sequence;

public:
  /**
   * The type of values stored in Container.
   */
  using value_type = typename Container::value_type;

  /**
   * Constructor.
   *
   * @param depth Maximum length of the sequence.
   */
  explicit history_sequence(std::size_t depth);

  /**
   * Add a new observation.
   *
   * @param observation The observation.
   */
  void add(std::int64_t observation);

  /**
   * @return the maximum length of the sequence.
   */
  size_t size() const noexcept;

  /**
   * @return a read-only iterator to the beginning.
   */
  Container::const_iterator begin() const noexcept;

  /**
   * @return a read-only iterator to the end.
   */
  Container::const_iterator end() const noexcept;

  /**
   * @return an immutable reference to the last element.
   */
  Container::value_type const &back() const;

  /**
   * @return The hamming distance between history sequences.
   */
  std::int64_t distance(history_sequence const &hs);
};

/**
 * Maintains a table where each row consists of multiple observations and their associated frequency. Indexing
 * into the table is decoupled from this object so that the user can pick their own hash function.
 */
class history_table {
public:
  /**
   * Each row maps an observed stride to the number of times it occured (i.e., its frequency).
   */
  struct row {
    explicit row(history_sequence sequence) : pattern(std::move(sequence))
    {
    }

    history_sequence pattern;
    std::map<std::int64_t, std::uint64_t> counts;
  };

  // use a map in case indicies are not contiguous
  using Container = std::map<std::uint64_t, row>;

  Container rows;

public:
  /**
   * @return true if there are no rows in the table.
   */
  bool empty() const;

  /**
   * @return The number of rows in the table.
   */
  size_t size() const;

  /**
   * Increment an observation in the table.
   *
   * This will create a new row if it does not exist at the index.
   *
   * @param index The row in the table.
   * @param observation The value of the observation.
   * @param pattern The stride pattern.
   */
  void increment(std::uint64_t index, std::int64_t observation, history_sequence const &pattern);

  /**
   * Setup a row in the table with the given pattern and count.
   *
   * @param pattern The stride pattern.
   * @param observation The value of the observation.
   * @param count The number of times it was observed.
   */
  void set(history_sequence pattern, std::int64_t observation, std::uint64_t count);

  /**
   * @return a read-only iterator to the beginning.
   */
  Container::const_iterator begin() const noexcept;

  /**
   * @return a read-only iterator to the end.
   */
  Container::const_iterator end() const noexcept;
};

/**
 * Stores the number of occurences of a stride, based on previous stride history.
 */
class spc_table {
public:
  /**
   * Constructor.
   *
   * @param stride_depth The amount of stride history to consider at a time.
   */
  explicit spc_table(std::size_t stride_depth);

  /**
   * Update the data structures based on the accessed address.
   *
   * @param address The address that is being accessed.
   */
  void update(std::uint64_t address);

  /**
   * @return The number of strides considered in the stride history.
   */
  std::size_t stride_pattern_depth() const
  {
    return last_M_strides.size();
  }

  bool first_request = true;

  std::uint64_t start_address;

  std::uint64_t last_address;

  history_table stride_patterns;

private:
  history_sequence last_M_strides;
};
}

#endif //STM_CLONING_SPC_TABLE_HPP
