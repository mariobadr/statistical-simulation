
#include <iogem5/packet-trace.hpp>

#include "iogem5/packet-trace.hpp"

#include "packet.pb.h"

namespace iogem5 {

static constexpr std::uint32_t GEM5_MAGIC_NUMBER = 0x356d6567;
static constexpr std::uint64_t GEM5_DEFAULT_TICK_FREQ = 1000000000000;

packet_trace_reader::packet_trace_reader(std::istream &stream)
    : input_stream(stream, GEM5_MAGIC_NUMBER)
{
  ProtoMessage::PacketHeader header;
  if(!input_stream.read(&header)) {
    throw std::runtime_error("Could not read packet header from trace.");
  }

  tick_frequency = header.tick_freq();
  object_id = header.obj_id();
}

bool packet_trace_reader::read(packet *p)
{
  ProtoMessage::Packet proto_packet;
  if(input_stream.read(&proto_packet)) {
    // Required fields.
    p->tick = proto_packet.tick();
    p->command = proto_packet.cmd();
    p->address = proto_packet.addr();
    p->size = proto_packet.size();

    // Optional fields.

    if(proto_packet.has_flags()) {
      p->flags = proto_packet.flags();
    }

    if(proto_packet.has_pkt_id()) {
      p->packet_id = proto_packet.pkt_id();
    }

    if(proto_packet.has_pc()) {
      p->pc = proto_packet.pc();
    }

    return true;
  }

  return false;
}

std::uint64_t packet_trace_reader::get_tick_frequency() const
{
  return tick_frequency;
}

std::string packet_trace_reader::get_object_id() const
{
  return object_id;
}

packet_trace_writer::packet_trace_writer(std::string const &file_name, std::uint64_t tick_freq)
    : output_stream(file_name, GEM5_MAGIC_NUMBER)
{
  ProtoMessage::PacketHeader header;
  header.set_obj_id("iogem5");
  header.set_tick_freq(tick_freq);

  output_stream.write(header);
}

packet_trace_writer::packet_trace_writer(std::string const &file_name)
    : packet_trace_writer(file_name, GEM5_DEFAULT_TICK_FREQ)
{
}

void packet_trace_writer::write(packet const &p)
{
  ProtoMessage::Packet proto_packet;

  proto_packet.set_tick(p.tick);
  proto_packet.set_cmd(p.command);
  proto_packet.set_addr(p.address);
  proto_packet.set_size(p.size);

  proto_packet.set_flags(p.flags);
  proto_packet.set_pkt_id(p.packet_id);
  proto_packet.set_pc(p.pc);

  output_stream.write(proto_packet);
}

void packet_trace_writer::write(std::uint64_t tick,
    std::uint32_t command,
    std::uint64_t address,
    std::uint32_t size)
{
  ProtoMessage::Packet proto_packet;

  proto_packet.set_tick(tick);
  proto_packet.set_cmd(command);
  proto_packet.set_addr(address);
  proto_packet.set_size(size);

  output_stream.write(proto_packet);
}

void packet_trace_writer::write(std::uint64_t tick,
    std::uint32_t command,
    std::uint64_t address,
    std::uint32_t size,
    std::uint64_t pc)
{
  ProtoMessage::Packet proto_packet;

  proto_packet.set_tick(tick);
  proto_packet.set_cmd(command);
  proto_packet.set_addr(address);
  proto_packet.set_size(size);
  proto_packet.set_pc(pc);

  output_stream.write(proto_packet);
}

} // namespace iogem5