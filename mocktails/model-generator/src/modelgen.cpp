#include "modelgen.hpp"

#include <fstream>
#include <vector>

#include <iogem5/packet-trace.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#include "mocktails/profile.hpp"
#include "mocktails/metadata.hpp"

#include "config.hpp"

namespace mocktails {

static constexpr std::uint32_t GEM5_MAGIC_NUMBER = 0x356d6567;

void write(ioproto::ofstream &output, mocktails::profile const &p)
{
  mocktails::append(output, p);
  spdlog::get("log")->info("Metadata for phase has been written to the output.");
}

void populate_hierarchy(hierarchy *h,
    std::uint32_t node_id,
    hierarchy::configuration const &config,
    std::size_t level_id)
{
  auto &partition = h->get_partition(node_id);
  auto &levels = config.levels;

  auto const event_count = partition.requests.size();
  auto const scheme = to_string(levels[level_id].scheme);

  if(level_id == levels.size() || event_count == 1) {
    spdlog::get("log")->debug(
        "Leaf node {} at level {} has {} event(s) that start at {} and span(s) {} cycles.", node_id,
        level_id - 1, event_count, partition.start_time, partition.duration);

    return;
  }

  spdlog::get("log")->debug("Node {} at level {} has {} event(s) that span(s) {} cycles.", node_id,
      level_id - 1, event_count, partition.duration);

  auto partitioning = split(&partition, levels[level_id]);
  spdlog::get("log")->debug(
      "Node {} split into {} {} partition(s).", node_id, partitioning.size(), scheme);

  for(auto &child : partitioning) {
    auto const child_id = h->add_partition(node_id, child.second);
    populate_hierarchy(h, child_id, config, level_id + 1);
  }
}

mocktails::profile generate_profile(std::uint32_t id,
    mocktails::partition p,
    hierarchy::configuration const &config,
    mocktails::model_type model_type)
{
  // Create a hierarchy of nodes out of the root partition.
  spdlog::get("log")->info("Partitioning {} requests into a hierarchy.", p.requests.size());
  hierarchy h(config, std::move(p));
  populate_hierarchy(&h, h.root_id(), config, 1);

  spdlog::get("log")->info("Modelling each leaf node of the hierarchy.");
  mocktails::profile profile(id, model_type, h);

  return profile;
}

void generate_model(std::string const &input_filename,
    std::string const &output_filename,
    std::string const &config_filename,
    std::string const &type,
    std::uint64_t root_size)
{
  auto const config = parse_configuration(config_filename);
  auto const model_type = parse_model_type(type);

  std::ifstream input_file(input_filename);
  iogem5::packet_trace_reader trace(input_file);
  spdlog::get("log")->info("Opened trace file: {}", input_filename);

  ioproto::ofstream output(output_filename, GEM5_MAGIC_NUMBER);
  spdlog::get("log")->info("Model will be written to {}.", output_filename);

  partition root;
  std::uint32_t profile_id = 0;

  // Loop through all the packets in the trace.
  iogem5::packet packet{};
  while(trace.read(&packet)) {

    // Map a gem5 read/write to a mocktails operation.
    operation op;
    if(packet.command == 1) {
      op = operation::read;
    } else if(packet.command == 4) {
      op = operation::write;
    } else {
      throw std::runtime_error("Unknown operation from trace.");
    }

    // Update the root partition.
    root.requests.emplace_back(packet.tick, op, packet.address, packet.size);
    root.duration = packet.tick - root.start_time;

    if(root_size > 0 && root.requests.size() % root_size == 0) {
      // Create a hierarchy of nodes out of the root partition.
      auto profile = generate_profile(profile_id, std::move(root), config, model_type);

      write(output, profile);

      // Recreate the root partition.
      root = partition{};
      root.start_time = packet.tick;

      profile_id++;
      spdlog::get("log")->info("{} execution phases have been modelled.", profile_id);
    }
  }

  if(!root.requests.empty()) {
    // Create a hierarchy of nodes out of the root partition.
    auto profile = generate_profile(profile_id, std::move(root), config, model_type);

    write(output, profile);
  }
}
} // namespace mocktails
