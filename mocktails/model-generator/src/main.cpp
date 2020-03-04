#include <fstream>
#include <iostream>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#include "argagg.hpp"

#include "modelgen.hpp"

constexpr auto DEFAULT_LOG_LEVEL = 2;
constexpr auto DEFAULT_MODEL_TYPE = "mocktails";

argagg::parser create_command_line_interface()
{
  return {{{"help", {"-h", "--help"}, "Display help information.", 0},
      {"input", {"-i", "--input"}, "gem5 packet trace.", 1},
      {"output", {"-o", "--output"}, "Output file.", 1},
      {"config", {"-c", "--configuration"}, "Configuration file.", 1},
      {"log", {"--log-level"},
          "0:6 maps to {trace, debug, info, warn, error, critical, off} (default: 2).", 1},
      {"size", {"--max-root-size"},
          "Maximum number of requests at the root of a hierarchy (default: 100,000).", 1},
      {"type", {"-t", "--model-type"}, "The type of model to use (mocktails, stm, hrd).", 1}}};
}

void print_usage(std::ostream &stream, argagg::parser const &arguments)
{
  argagg::fmt_ostream help(stream);

  help << "Create Mocktails model from a memory trace.\n\n";
  help << "create-mocktails-profile [options] ARG [ARG...]\n\n";
  help << arguments;
}

void ensure_file_exists(std::string const &path_to_file)
{
  std::ifstream file(path_to_file);
  if(!file.good()) {
    throw std::runtime_error("The file " + path_to_file + " does not exist.");
  }
}

void validate(argagg::parser_results const &arguments)
{
  if(arguments["input"].count() == 0) {
    throw std::runtime_error("Missing path gem5 packet trace.");
  } else {
    ensure_file_exists(arguments["input"].as<std::string>());
  }

  if(arguments["output"].count() == 0) {
    throw std::runtime_error("Missing path to output file.");
  }

  if(arguments["config"].count() == 0) {
    throw std::runtime_error("Missing path to configuration file.");
  } else {
    ensure_file_exists(arguments["config"].as<std::string>());
  }

  auto const log_level = arguments["log"].as<int>(DEFAULT_LOG_LEVEL);
  if(log_level < 0 || log_level > 6) {
    throw std::runtime_error("Please provide a log level between 0 and 6.");
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

    auto const log_level = arguments["log"].as<int>(DEFAULT_LOG_LEVEL);
    spdlog::get("log")->set_level(static_cast<spdlog::level::level_enum>(log_level));

    auto const input_filename = arguments["input"].as<std::string>();
    auto const output_filename = arguments["output"].as<std::string>();
    auto const config_filename = arguments["config"].as<std::string>();
    auto const model_type = arguments["type"].as<std::string>(DEFAULT_MODEL_TYPE);

    // Set the length of an execution phase.
    auto const root_size = arguments["size"].as<std::uint64_t>(100000);

    // Generate the model.
    mocktails::generate_model(
        input_filename, output_filename, config_filename, model_type, root_size);
  } catch(std::exception const &e) {
    spdlog::get("log")->error("{}", e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}