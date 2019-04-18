#include "modelgen.hpp"

#include <algorithm>
#include <fstream>

#include <ioproto/ofstream.hpp>
#include <iogem5/packet-trace.hpp>
#include <spdlog/spdlog.h>

#include <hrd/metadata.hpp>

static constexpr std::uint32_t GEM5_MAGIC_NUMBER = 0x356d6567;

void generate_hrd_model(std::string const &input_filename,
    std::string const &output_filename,
    std::vector<std::uint64_t> layers)
{
  std::sort(layers.begin(), layers.end());

  spdlog::get("log")->info("{} layers have been configured and sorted.", layers.size());
  for(std::size_t i = 0; i < layers.size(); ++i) {
    spdlog::get("log")->info("Layer {} has a block size of {} bytes.", i, layers[i]);
  }

  std::ifstream input_file(input_filename);
  iogem5::packet_trace_reader trace(input_file);
  spdlog::get("log")->info("Opened trace file: {}", input_filename);

  hrd::profile model(layers);

  // Loop through all the packets in the trace.
  iogem5::packet packet{};
  while(trace.read(&packet)) {

    // Map a gem5 read/write to an HRD operation.
    hrd::operation operation;
    if(packet.command == 1) {
      operation = hrd::operation::read;
    } else if(packet.command == 4) {
      operation = hrd::operation::write;
    } else {
      throw std::runtime_error("Unknown operation from trace.");
    }

    // Update the statistical profile.
    model.update(packet.address, operation);

    if(model.count() % 1000000 == 0) {
      spdlog::get("log")->info("{} requests have been modelled so far ({} unique addresses).",
          model.count(), model.unique_addresses());
    }
  }

  spdlog::get("log")->info("{} requests have been modelled.", model.count());
  spdlog::get("log")->info("There were {} unique addresses in the range {} to {}.",
      model.unique_addresses(), model.min_address, model.max_address);

  if(model.count() > 0) {
    spdlog::get("log")->info("Model will be written to {}.", output_filename);
    ioproto::ofstream output(output_filename, GEM5_MAGIC_NUMBER);

    hrd::append(output, model);
    spdlog::get("log")->info("The model metadata has been written to the output.");
  }
}
