#include <iostream>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#include "argagg.hpp"
#include "tracegen.hpp"

argagg::parser create_command_line_interface()
{
  return {{{"help", {"-h", "--help"}, "Display help information.", 0},
      {"input", {"-i", "--input"}, "HRD statistical profile.", 1},
      {"output", {"-o", "--output"}, "Output file.", 1}}};
}

void print_usage(std::ostream &stream, argagg::parser const &arguments)
{
  argagg::fmt_ostream help(stream);

  help << "Create a trace from an HRD model.\n\n";
  help << "create-hrd-trace [options] ARG [ARG...]\n\n";
  help << arguments;
}

void validate(argagg::parser_results const &options)
{
  if(options["input"].count() == 0) {
    throw std::runtime_error("Missing path to HRD statistical profile.");
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

    generate_trace(input_filename, output_filename);
  } catch(std::exception const &e) {
    spdlog::get("log")->error("{}", e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
