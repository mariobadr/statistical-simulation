#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#include "argagg.hpp"
#include "modelgen.hpp"

argagg::parser create_command_line_interface()
{
  return {{{"help", {"-h", "--help"}, "Display help information.", 0},
      {"input", {"-i", "--input"}, "gem5 packet trace.", 1},
      {"output", {"-o", "--output"}, "Output file.", 1},
      {"layers", {"-l", "--layers"}, "Layers of the hierarchy (default: 64,4096)", 1}}};
}

void print_usage(std::ostream &stream, argagg::parser const &arguments)
{
  argagg::fmt_ostream help(stream);

  help << "Create an HRD model from a memory trace.\n\n";
  help << "create-hrd-profile [options] ARG [ARG...]\n\n";
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

std::vector<std::uint64_t> parse_layers(std::string const &layers_argument)
{
  std::stringstream stream(layers_argument);
  std::string substring;

  std::vector<std::uint64_t> layers;

  while(std::getline(stream, substring, ',')) {
    std::uint64_t const layer = std::stoull(substring);

    layers.push_back(layer);
  }

  return layers;
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

    // Parse the layers from a comma-separated string (no spaces).
    auto layers = parse_layers(arguments["layers"].as<std::string>("64,4096"));

    // Generate the model.
    generate_hrd_model(input_filename, output_filename, std::move(layers));
  } catch(std::exception const &e) {
    spdlog::get("log")->error("{}", e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}