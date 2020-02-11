#include "mocktails/partition.hpp"

#include <cassert>
#include <cmath>
#include <mocktails/partition.hpp>

namespace mocktails {

partition::partition(configuration config) : m_config(config)
{
}

std::map<std::uint32_t, partition> create_fixed_time_partitions(partition const &p,
    partition::configuration configuration)
{
  std::map<std::uint32_t, partition> result;
  auto const &resolution = configuration.separator_value;

  for(auto const &req : p.requests) {
    // Anchor the timestamps based on this partition's start time.
    auto const time = req.timestamp - p.start_time;
    auto const id = static_cast<std::uint32_t>(time / resolution);

    auto emplaced = result.emplace(id, configuration);
    auto &new_partition = emplaced.first->second;

    if(emplaced.second) {
      // This partition was just created.
      new_partition.start_time = p.start_time + (id * resolution);
      new_partition.duration = resolution;
    }

    // Move the requests from p to the new partitions we are creating.
    new_partition.requests.emplace_back(req);
  }

  return result;
}

std::map<std::uint32_t, partition> create_fixed_number_of_partitions(partition const &p,
    partition::configuration configuration)
{
  auto const total_time = static_cast<double>(p.duration);
  auto const partition_count = static_cast<double>(configuration.separator_value);
  auto const resolution = std::ceil((total_time + 1.0) / partition_count);

  configuration.separator = partition::delimiter::cycles;
  configuration.separator_value = static_cast<std::uint32_t>(resolution);

  return create_fixed_time_partitions(p, configuration);
}

std::map<std::uint32_t, partition> create_fixed_requests_partitions(partition const &p,
    partition::configuration configuration)
{
  std::map<std::uint32_t, partition> result;

  std::size_t const size = p.requests.size() / configuration.separator_value + 1;

  auto start_it = p.requests.begin();
  for(std::size_t i = 0; i < size; ++i) {
    auto end_it = start_it;
    if(i + 1 < size) {
      end_it = std::next(end_it, configuration.separator_value);
    } else {
      end_it = p.requests.end();
    }

    auto emplaced = result.emplace(i, configuration);
    assert(emplaced.second);
    auto &new_partition = emplaced.first->second;

    // Move the requests from p to the new partitions we are creating.
    std::move(start_it, end_it, std::back_inserter(new_partition.requests));

    // Update times.
    new_partition.start_time = new_partition.requests.front().timestamp;
    new_partition.duration = new_partition.requests.back().timestamp - new_partition.start_time;

    start_it = end_it;
  }
  assert(start_it == p.requests.end());

  return result;
}

std::map<std::uint32_t, partition> create_fixed_byte_partitions(partition const &p,
    partition::configuration configuration)
{
  std::map<std::uint32_t, partition> result;

  auto const &block_size = configuration.separator_value;
  for(auto const &req : p.requests) {
    // Anchor the timestamps based on this partition's start time.
    auto const block = std::floor(req.address / block_size);
    auto const id = static_cast<std::uint32_t>(block);

    auto emplaced = result.emplace(id, configuration);
    auto &new_partition = emplaced.first->second;

    if(emplaced.second) {
      // This partition was just created.
      new_partition.start_time = req.timestamp;
    }

    new_partition.duration = req.timestamp - new_partition.start_time;
    // Move the requests from p to the new partitions we are creating.
    new_partition.requests.emplace_back(req);
  }

  return result;
}

std::map<address_t, address_range> get_contiguous_ranges(partition const &p)
{
  // Turn every request into an address range, sorted by start address.
  std::map<address_t, address_range> ranges;

  for(auto const &req : p.requests) {
    auto const start_address = req.address;
    auto const end_address = req.address + req.size;

    ranges.emplace(start_address, address_range{start_address, end_address, 1});
  }

  // Combine the address ranges that are contiguous in memory.
  std::map<address_t, address_range> contiguous_ranges;
  address_range grouped_range = ranges.begin()->second;

  for(auto it = std::next(ranges.begin()); it != ranges.end(); ++it) {
    bool intersects = it->second.intersects(grouped_range);
    if(intersects) {
      // continuity, update end of the address range for the next iteration
      grouped_range.expand(it->second);
      grouped_range.count++;
    } else {
      // discontinuity
      contiguous_ranges.emplace(grouped_range.start, grouped_range);
      grouped_range = it->second;
    }
  }

  // Save the last group.
  contiguous_ranges.emplace(grouped_range.start, grouped_range);

  return contiguous_ranges;
}

using range_it = std::map<address_t, address_range>::iterator;

range_it combine_range(range_it it, range_it next, range_it end, stride_t last_stride)
{
  if(next == end) {
    return std::prev(next);
  }

  if(next->second.count > 1) {
    return next;
  }

  auto const stride = calculate_stride(next->first, it->second.end);

  if(stride != last_stride) {
    return next;
  }

  it->second.expand(next->second);
  it->second.count++;

  return combine_range(it, std::next(next), end, last_stride);
}

std::map<std::uint32_t, partition> create_contiguous_address_partitions(partition const &p,
    partition::configuration configuration)
{
  // Get the contiguous address ranges based on the requests in the partition.
  auto ranges = get_contiguous_ranges(p);

  // Combine address ranges that contain only one request.
  for(auto it = std::next(ranges.begin()); it != ranges.end(); ++it) {
    if(std::prev(it)->second.count == 1) {
      auto const stride = calculate_stride(it->first, std::prev(it)->second.end);
      it = combine_range(std::prev(it), it, ranges.end(), stride);
    }
  }

  // Create as many partitions as their are address ranges.
  std::map<std::uint32_t, partition> result;

  for(auto const &req : p.requests) {
    int id = 0;

    for(auto const &range : ranges) {
      if(range.second.contains(req.address)) {
        auto emplaced = result.emplace(id, configuration);
        auto &new_partition = emplaced.first->second;

        if(emplaced.second) {
          // This partition was just created.
          new_partition.start_time = req.timestamp;
        }

        new_partition.duration = req.timestamp - new_partition.start_time;

        // Move the requests from p to the new partitions we are creating.
        new_partition.requests.emplace_back(req);

        // There should be no other range this request belongs to.
        break;
      }

      id++;
    }
  }

  return result;
}

std::map<std::uint32_t, partition> split(partition *parent, partition::configuration configuration)
{
  if(parent->requests.empty()) {
    return {};
  }

  std::map<uint32_t, partition> result;

  if(configuration.scheme == partition::type::temporal) {
    if(configuration.separator == partition::delimiter::cycles) {
      result = create_fixed_time_partitions(*parent, configuration);
    } else if(configuration.separator == partition::delimiter::count) {
      result = create_fixed_number_of_partitions(*parent, configuration);
    } else if(configuration.separator == partition::delimiter::requests) {
      result = create_fixed_requests_partitions(*parent, configuration);
    }
  } else if(configuration.scheme == partition::type::spatial) {
    if(configuration.separator == partition::delimiter::contiguous) {
      result = create_contiguous_address_partitions(*parent, configuration);
    } else if(configuration.separator == partition::delimiter::bytes) {
      result = create_fixed_byte_partitions(*parent, configuration);
    }
  }

  // Remove the requests from the parent partition, they are no longer needed.
  parent->requests.clear();
  parent->requests.shrink_to_fit();

  return result;
}

std::string to_string(partition::type scheme)
{
  switch(scheme) {
  case mocktails::partition::type::spatial:
    return "spatial";
  case mocktails::partition::type::temporal:
    return "temporal";
  }

  return "unknown";
}
} // namespace mocktails