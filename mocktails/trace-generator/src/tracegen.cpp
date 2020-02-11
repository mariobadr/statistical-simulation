#include "tracegen.hpp"

#include <fstream>

#include <spdlog/spdlog.h>

#include <iogem5/packet-trace.hpp>

#include <mocktails/metadata.hpp>
#include <mocktails/synthesis.hpp>

static constexpr std::uint32_t GEM5_MAGIC_NUMBER = 0x356d6567;

void generate_trace(std::string const &input_filename, std::string const &output_filename)
{
  spdlog::get("log")->info("Loading statistical profile from: {}.", input_filename);

  std::ifstream stream(input_filename);
  ioproto::istream input(stream, GEM5_MAGIC_NUMBER);

  std::uint64_t total_count = 0;
  auto profile = mocktails::read(input);

  iogem5::packet_trace_writer trace(output_filename);
  spdlog::get("log")->info("Synthetic trace will be written to {}.", output_filename);

  while(profile != nullptr) {
    mocktails::synthesiser synthesiser(*profile);
    profile = mocktails::read(input);

    std::uint64_t request_count = 0;
    while(synthesiser.has_more_requests()) {
      auto const request = synthesiser.generate_next_request();
      request_count++;

      iogem5::packet packet{};
      packet.tick = request.timestamp;
      packet.address = request.address;
      packet.size = static_cast<std::uint32_t>(request.size);

      if(request.op == mocktails::operation::read) {
        packet.command = 1;
      } else {
        packet.command = 4;
      }

      trace.write(packet);
    }

    total_count += request_count;
    spdlog::get("log")->info("{} requests have been synthesized so far.", total_count);
  }

  spdlog::get("log")->info("Generated {} requests.", total_count);
}
