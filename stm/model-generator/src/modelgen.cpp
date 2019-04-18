#include "modelgen.hpp"

#include <fstream>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "iogem5/packet-trace.hpp"

#include "stm/metadata.hpp"

static constexpr std::uint32_t GEM5_MAGIC_NUMBER = 0x356d6567;

void write(ioproto::ofstream &output, stm::profile const &p)
{
  spdlog::get("log")->info("{}-request phase modelled.", p.count());
  stm::append(output, p);
  spdlog::get("log")->info("Metadata for phase has been written to the output.");
}

void generate_stm_model(std::string const &input_filename,
    std::string const &output_filename,
    stm::profile::parameters const &parameters,
    std::uint64_t interval_size)
{
  spdlog::get("log")->info("SDC Rows: {}", parameters.num_rows);
  spdlog::get("log")->info("SDC Columns: {}", parameters.num_cols);
  spdlog::get("log")->info("Stride Depth: {}", parameters.stride_depth);
  spdlog::get("log")->info("Interval Size: {}", interval_size);

  std::ifstream input_file(input_filename);
  iogem5::packet_trace_reader trace(input_file);
  spdlog::get("log")->info("Opened trace file: {}", input_filename);

  ioproto::ofstream output(output_filename, GEM5_MAGIC_NUMBER);
  spdlog::get("log")->info("Model will be written to {}.", output_filename);

  stm::profile model(parameters);
  int interval_count = 0;

  // Loop through all the packets in the trace.
  iogem5::packet packet{};
  while(trace.read(&packet)) {

    // Map a gem5 read/write to an STM operation.
    stm::operation operation;
    if(packet.command == 1) {
      operation = stm::operation::read;
    } else if(packet.command == 4) {
      operation = stm::operation::write;
    } else {
      throw std::runtime_error("Unknown operation from trace.");
    }

    // Update the statistical profile.
    model.update(packet.address, operation);

    if(model.count() % interval_size == 0) {
      write(output, model);

      // Create a new STM profile to populate.
      model = stm::profile(parameters);

      interval_count++;
      spdlog::get("log")->info("{} execution phases have been modelled.", interval_count);
    }
  }

  // Make sure the last execution phase is outputted.
  if(model.count() > 0) {
    write(output, model);
  }
}
