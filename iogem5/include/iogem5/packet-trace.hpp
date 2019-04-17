#ifndef IOGEM5_PACKET_TRACE_HPP
#define IOGEM5_PACKET_TRACE_HPP

#include <iosfwd>
#include <string>
#include <memory>

#include <ioproto/istream.hpp>
#include <ioproto/ofstream.hpp>

namespace iogem5 {

/**
 * Represents a gem5 packet.
 */
struct packet {
  std::uint64_t tick = 0;
  std::uint32_t command = 0;
  std::uint64_t address = 0;
  std::uint32_t size = 0;
  std::uint32_t flags = 0;
  std::uint64_t packet_id = 0;
  std::uint64_t pc = 0;
};

/**
 * Responsible for reading a gem5 packet trace.
 */
class packet_trace_reader {
public:
  /**
   * Constructor.
   */
  explicit packet_trace_reader(std::istream &stream);

  /**
   * Read a packet from the trace and populate p with the data.
   *
   * @param p The packet to populate.
   *
   * @return true if a packet was read, false otherwise (e.g., EOF).
   */
  bool read(packet *p);

  /**
   * @return The frequency of a single tick in the trace.
   */
  std::uint64_t get_tick_frequency() const;

  /**
   * @return The identifier associated with the trace.
   */
  std::string get_object_id() const;

private:
  ioproto::istream input_stream;

  std::uint64_t tick_frequency;
  std::string object_id;
};

/**
 * Responsible for writing a gem5 packet trace.
 */
class packet_trace_writer {
public:
  /**
   * Constructor.
   *
   * Opens a file for writing a gem5 packet trace.
   */
  packet_trace_writer(std::string const &file_name, std::uint64_t tick_freq);

  /**
   * Constructor.
   *
   * Opens a file for writing a gem5 packet trace, using the default gem5 tick frequency.
   */
  explicit packet_trace_writer(std::string const &file_name);

  /**
   * Write all the fields of a packet to the file.
   */
  void write(packet const &p);

  /**
   * Write only the required fields of a packet to the file.
   */
  void write(std::uint64_t tick, std::uint32_t command, std::uint64_t address, std::uint32_t size);

  /**
   * Write only the required fields, and the optional pc, of a packet to the file.
   */
  void write(std::uint64_t tick,
      std::uint32_t command,
      std::uint64_t address,
      std::uint32_t size,
      std::uint64_t pc);

private:
  ioproto::ofstream output_stream;
};

} // namespace iogem5

#endif // IOGEM5_PACKET_TRACE_HPP
