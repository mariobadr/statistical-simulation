#include "tracegen.hpp"

#include <fstream>

#include "spdlog/spdlog.h"
#include "iogem5/packet-trace.hpp"

#include "stm/metadata.hpp"
#include "stm/synthesis.hpp"

static constexpr std::uint32_t GEM5_MAGIC_NUMBER = 0x356d6567;

void generate_trace(std::string const &input_filename, std::string const &output_filename)
{
  spdlog::get("log")->info("Loading statistical profile from: {}.", input_filename);

  std::ifstream stream(input_filename);
  ioproto::istream input(stream, GEM5_MAGIC_NUMBER);

  std::uint64_t total_count = 0;
  auto profile = stm::read(input);

  iogem5::packet_trace_writer trace(output_filename);
  spdlog::get("log")->info("Synthetic trace will be written to {}.", output_filename);

  while(profile != nullptr) {
    std::uint64_t const request_count = profile->count();
    stm::synthesiser synthesiser(std::move(*profile));
    profile = stm::read(input);

    for(std::uint64_t i = 0; i < request_count; i++) {
      iogem5::packet packet{};
      packet.tick = i;
      packet.size = 32; // assume 32 byte requests

      stm::operation operation;
      synthesiser.generate_next_request(packet.address, operation);

      if(operation == stm::operation::read) {
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
