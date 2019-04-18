#include "tracegen.hpp"

#include <fstream>

#include <ioproto/ofstream.hpp>
#include <iogem5/packet-trace.hpp>
#include <spdlog/spdlog.h>

#include <hrd/synthesis.hpp>
#include <hrd/metadata.hpp>

static constexpr std::uint32_t GEM5_MAGIC_NUMBER = 0x356d6567;

void generate_trace(std::string const &input_filename, std::string const &output_filename)
{
  spdlog::get("log")->info("Loading statistical profile from: {}.", input_filename);
  std::ifstream file_stream(input_filename);
  ioproto::istream input(file_stream, GEM5_MAGIC_NUMBER);

  std::uint64_t request_count;
  auto profile = hrd::read(input, &request_count);
  spdlog::get("log")->info("Successfully loaded statistical profile ({} requests).", request_count);

  iogem5::packet_trace_writer trace(output_filename);
  spdlog::get("log")->info("Synthetic trace will be written to {}.", output_filename);

  hrd::synthesiser synthesiser(profile);

#ifdef HRD_VALIDATE_TRACE
  hrd::profile validator(profile.layers);
#endif

  for(std::uint64_t i = 1; i <= request_count; i++) {
    iogem5::packet packet{};
    packet.tick = i;
    packet.size = static_cast<std::uint32_t>(profile.layers[0]);

    hrd::operation operation;
    synthesiser.generate_next_request(packet.address, operation);

    if(operation == hrd::operation::read) {
      packet.command = 1;
    } else {
      packet.command = 4;
    }

    trace.write(packet);

#ifdef HRD_VALIDATE_TRACE
    validator.update(packet.address, operation);
#endif

    if(i % 10000 == 0) {
      spdlog::get("log")->info("{} requests have been synthesized so far.", i);
    }
  }

  spdlog::get("log")->info("Successfully generated {} requests.", request_count);

#ifdef HRD_VALIDATE_TRACE
  spdlog::get("log")->info("Trace validation was enabled.");
  spdlog::get("log")->info(
      "Minimum Address - Profile: {} | Validator {}.", profile.min_address, validator.min_address);
  spdlog::get("log")->info(
      "Maximum Address - Profile: {} | Validator {}.", profile.max_address, validator.max_address);

  spdlog::get("log")->info("Profile Reuse Histograms");
  std::size_t layer = 0;
  for(auto const &hist : profile.reuse_model) {
    for(auto const &pair : hist) {
      spdlog::get("log")->info(
          "Layer: {}, Distance: {}, Count: {}", layer, pair.first, pair.second);
    }

    layer++;
  }

  spdlog::get("log")->info("Validator Reuse Histograms");
  layer = 0;
  for(auto const &hist : validator.reuse_model) {
    for(auto const &pair : hist) {
      spdlog::get("log")->info(
          "Layer: {}, Distance: {}, Count: {}", layer, pair.first, pair.second);
    }

    layer++;
  }
#endif
}
