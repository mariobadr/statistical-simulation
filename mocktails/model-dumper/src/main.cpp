#include <iostream>

#include "ioproto/istream.hpp"
#include "mocktails/metadata.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"

#include "argagg.hpp"

static constexpr std::uint32_t GEM5_MAGIC_NUMBER = 0x356d6567;

argagg::parser create_command_line_interface()
{
  return {{{"help", {"-h", "--help"}, "Display help information.", 0},
      {"input", {"-i", "--input"}, "gem5 packet trace.", 1},
      {"output", {"-o", "--output"}, "Output file.", 1}}};
}

void print_usage(std::ostream &stream, argagg::parser const &arguments)
{
  argagg::fmt_ostream help(stream);

  help << "Dump an Mocktails model to a csv file.\n\n";
  help << "dump-mocktails-profile [options] ARG [ARG...]\n\n";
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

void dump_to_csv(std::string const &input_filename, std::string const &output_filename)
{
  spdlog::get("log")->info("Loading statistical profile from: {}.", input_filename);
  std::ifstream file_stream(input_filename);
  ioproto::istream input(file_stream, GEM5_MAGIC_NUMBER);

  auto profile = mocktails::read(input);
  if(profile->type != mocktails::model_type::mocktails) {
    throw std::runtime_error("model-dumper only works with simple leaves.");
  }

  std::ofstream output(output_filename);
  output << "profile.id,node.id,total,min.address,max.address,start.address,size.states,time."
            "states,stride.states,op.states\n";

  std::size_t profile_id = 0;
  while(profile != nullptr) {
    spdlog::get("log")->info(
        "Successfully loaded statistical profile ({} nodes).", profile->simple_leaves.size());

    for(auto const &pair : profile->simple_leaves) {
      auto const &model = pair.second;

      output << profile_id << ",";
      output << pair.first << ",";
      output << model.request_count << ",";
      output << model.underlying_model->footprint.start << ",";
      output << model.underlying_model->footprint.end << ",";
      output << model.underlying_model->start_address << ",";
      output << model.size_model.transitions.size() << ",";
      output << model.time_model.transitions.size() << ",";
      output << model.underlying_model->stride_model.transitions.size() << ",";
      output << model.underlying_model->operation_model.transitions.size() << std::endl;
    }

    profile_id++;
    profile = mocktails::read(input);
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

    dump_to_csv(input_filename, output_filename);
  } catch(std::exception const &e) {
    spdlog::get("log")->error("{}", e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
