#include <fstream>
#include <iostream>
#include <vector>

#include "argagg.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "stm/profile.hpp"

#include "modelgen.hpp"

argagg::parser create_command_line_interface()
{
  return {{{"help", {"-h", "--help"}, "Display help information.", 0},
      {"input", {"-i", "--input"}, "gem5 packet trace.", 1},
      {"output", {"-o", "--output"}, "Output file.", 1},
      {"rows", {"--num-rows"}, "Number of rows in the SDC table (default: 128).", 1},
      {"cols", {"--num-cols"}, "Number of columns in the SDC table (default: 2).", 1},
      {"depth", {"--max-stride-depth"}, "Maximum stride history to track (default: 80).", 1},
      {"interval", {"--interval-size"},
          "Maximum number of requests in an execution phase (default: 100,000).", 1}}};
}

void print_usage(std::ostream &stream, argagg::parser const &arguments)
{
  argagg::fmt_ostream help(stream);

  help << "Create an STM model from a memory trace.\n\n";
  help << "create-stm-profile [options] ARG [ARG...]\n\n";
  help << arguments;
}

void validate(argagg::parser_results const &options)
{
  if(options["input"].count() == 0) {
    throw std::runtime_error("Missing path to gem5 packet trace.");
  }

  if(options["output"].count() == 0) {
    throw std::runtime_error("Missing path to output file.");
  }
}

int main(int argc, char **argv)
{
  try {
    // Create the logger.
    spdlog::stdout_logger_st("log");

    // Parse the command line.
    auto interface = create_command_line_interface();
    auto const arguments = interface.parse(argc, argv);

    // Check if help was requested.
    if(arguments["help"]) {
      print_usage(std::cout, interface);

      return EXIT_SUCCESS;
    }

    // Make sure we have the required arguments.
    validate(arguments);

    auto const input_filename = arguments["input"].as<std::string>();
    auto const output_filename = arguments["output"].as<std::string>();

    // Setup the STM parameters.
    // The default values are from the paper.
    stm::profile::parameters parameters{};
    parameters.num_rows = arguments["rows"].as<std::size_t>(128);
    parameters.num_cols = arguments["cols"].as<std::size_t>(2);
    parameters.stride_depth = arguments["depth"].as<std::size_t>(80);

    // Set the length of an execution phase.
    auto const interval_size = arguments["interval"].as<std::uint64_t>(100000);

    // Generate the model.
    generate_stm_model(input_filename, output_filename, parameters, interval_size);
  } catch(std::exception const &e) {
    spdlog::get("log")->error("{}", e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}