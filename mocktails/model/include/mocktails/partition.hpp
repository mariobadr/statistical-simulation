#ifndef MOCKTAILS_PARTITION_HPP
#define MOCKTAILS_PARTITION_HPP

#include <map>
#include <string>
#include <vector>

#include "mocktails/request.hpp"

namespace mocktails {

/**
 * A collection of memory requests.
 */
class partition {
public:
  /**
   * Supported partitioning schemes.
   */
  enum class type {
    /** Partition based on the when a request occured. */
    temporal,
    /** Partition based on the addresses of requests. */
    spatial
  };

  /**
   * Supported delimiters for the different partitioning schemes.
   */
  enum class delimiter {
    /** A fixed cycle count (temporal). */
    cycles,
    /** A fixed number of requests (temporal). */
    requests,
    /** A fixed number of partitions (temporal). */
    count,
    /** Contiguous address ranges (spatial). */
    contiguous,
    /** Fixed-size address ranges (spatial). */
    bytes
  };

  /**
   * The configuration of a partition.
   */
  struct configuration {
    /**
     * The default configuration is a monolithic temporal partition.
     */
    configuration() : scheme(type::temporal), separator(delimiter::count), separator_value(1)
    {
    }

    /**
     * Constructor.
     */
    configuration(type t, delimiter d, std::uint32_t sep)
        : scheme(t), separator(d), separator_value(sep)
    {
    }

    type scheme;
    delimiter separator;
    std::uint32_t separator_value;
  };

public:
  /**
   * Create a partition with the default configuration.
   */
  partition() = default;

  /**
   * Create a partition with the specified configuration.
   */
  explicit partition(configuration config);

public:
  std::vector<request> requests;
  std::uint64_t start_time = 0;
  std::uint64_t duration = 0;

private:
  configuration m_config;
};

/**
 * Split the partition in the temporal/spatial dimension.
 *
 * The parent partition will have its requests moved to its children partitions.
 */
std::map<std::uint32_t, partition> split(partition *parent, partition::configuration configuration);

/**
 * Convert the enum into a string.
 */
std::string to_string(partition::type scheme);
}

#endif //MOCKTAILS_PARTITION_HPP
