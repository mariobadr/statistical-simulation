#include <fstream>
#include <iostream>

#include "argagg.hpp"

#include <iogem5/packet-trace.hpp>

argagg::parser create_command_line_interface()
{
  return {{{"help", {"-h", "--help"}, "Display help information.", 0},
      {"input", {"-i", "--input"}, "gem5 packet trace.", 1},
      {"output", {"-o", "--output"}, "Output file.", 1},
      {"size", {"--max-size"}, "Maximum number of requests of the truncated trace.", 1}}};
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

  if(arguments["size"].count() == 0) {
    throw std::runtime_error("Unknown maximum number of requests..");
  }
}

void truncate_trace(std::string const &input_filename,
    std::string const &output_filename,
    int maximum_size)
{
  std::ifstream input_file(input_filename);

  iogem5::packet_trace_reader reader(input_file);
  iogem5::packet_trace_writer writer(output_filename, reader.get_tick_frequency());

  int count = 0;
  iogem5::packet packet{};
  while(count < maximum_size && reader.read(&packet)) {
    writer.write(packet);
    count++;
  }

  std::cout << "Wrote " << count << " packets from " << input_filename << " to " << output_filename
            << std::endl;
}

int main(int argc, char **argv)
{
  try {
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
    auto const maximum_size = arguments["size"].as<int>();

    truncate_trace(input_filename, output_filename, maximum_size);
  } catch(std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}