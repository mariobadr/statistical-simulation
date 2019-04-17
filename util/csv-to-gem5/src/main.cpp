#include <iostream>
#include <fstream>

#include "iogem5/packet-trace.hpp"
#include "argagg.hpp"

argagg::parser create_command_line_interface()
{
  return {{{"help", {"-h", "--help"}, "Display help information.", 0},
      {"input", {"-i", "--input"}, "Pin tool generated trace.", 1},
      {"output", {"-o", "--output"}, "Name of output file.", 1},
      {"cpi", {"--cpi"}, "Cycles-per-instruction to determine ticks.", 1},
      {"freq", {"--sim-freq"}, "The SimClock::frequency for gem5.", 1}}};
}

void print_usage(std::ostream &stream, argagg::parser const &arguments)
{
  argagg::fmt_ostream help(stream);

  help << "Create gem5 packet trace from the Pin tool generated trace.\n\n";
  help << "convert-to-gem5 [options]\n\n";
  help << arguments;
}

std::vector<std::string> tokenize(std::istream &stream)
{
  std::vector<std::string> tokens;
  std::string line;

  if(std::getline(stream, line)) {
    std::stringstream line_stream(line);
    std::string token;

    while(std::getline(line_stream, token, ',')) {
      tokens.emplace_back(token);
    }
  }

  return tokens;
}

void convert_to_gem5(std::string const &input_filename,
    std::string const &output_filename,
    double cpi,
    std::uint64_t sim_freq)
{
  constexpr std::size_t EXPECTED_COLUMNS = 5;
  std::ifstream input(input_filename);

  std::vector<std::string> tokens = tokenize(input);
  if(tokens.size() != EXPECTED_COLUMNS) {
    throw std::runtime_error("Unexpected number of columns.");
  }

  auto const first_count = std::stol(tokens[0]);
  int serialized_count = 0;

  iogem5::packet_trace_writer trace_writer(output_filename, sim_freq);
  while(tokens.size() == EXPECTED_COLUMNS) {
    iogem5::packet packet;

    auto const count = static_cast<double>(std::stol(tokens[0]) - first_count) * cpi;
    packet.tick = static_cast<std::uint64_t>(count);

    if(tokens[1] == "0") {
      packet.command = 1;
    } else if(tokens[1] == "1") {
      packet.command = 4;
    } else {
      throw std::runtime_error("Unexpected read/write operation.");
    }

    packet.address = std::stoull(tokens[2]);
    packet.size = static_cast<std::uint32_t>(std::stoul(tokens[3]));
    packet.pc = std::stoull(tokens[4]);

    trace_writer.write(packet);

    serialized_count++;
    if(serialized_count % 1000000 == 0) {
      std::cout << serialized_count << " packets serialized." << std::endl;
    }

    tokens = tokenize(input);
  }
}

int main(int argc, char **argv)
{
  try {
    auto interface = create_command_line_interface();
    auto const arguments = interface.parse(argc, argv);

    if(arguments["help"]) {
      print_usage(std::cout, interface);

      return EXIT_SUCCESS;
    }

    auto const input_filename = arguments["input"].as<std::string>();
    auto const output_filename = arguments["output"].as<std::string>();
    auto const cpi = arguments["cpi"].as<double>(1.0);
    auto const freq = arguments["freq"].as<std::uint64_t>(1000000000000);

    convert_to_gem5(input_filename, output_filename, cpi, freq);
  } catch(std::exception const &e) {
    std::cerr << "Error: " << e.what() << "\n";

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}