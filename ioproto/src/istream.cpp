#include "ioproto/istream.hpp"

#include <istream>

#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace ioproto {

using google::protobuf::Message;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::GzipInputStream;
using google::protobuf::io::IstreamInputStream;
using google::protobuf::io::ZeroCopyInputStream;

bool is_gzipped(std::istream &stream)
{
  bool use_gzip = false;

  unsigned char magic_number[2];
  if(stream >> magic_number[0]) {
    if(stream >> magic_number[1]) {
      use_gzip = magic_number[0] == 0x1f && magic_number[1] == 0x8b;
    }
  }

  // Reset the stream to its initial state.
  stream.clear();
  stream.seekg(0, std::istream::beg);

  return use_gzip;
}

istream::istream(std::istream &stream)
{
  parent_stream = std::make_unique<IstreamInputStream>(&stream);
  input_stream = parent_stream.get();

  if(is_gzipped(stream)) {
    gzip_stream = std::make_unique<GzipInputStream>(parent_stream.get());
    input_stream = gzip_stream.get();
  }
}

istream::istream(std::istream &stream, std::uint32_t magic_number) : istream(stream)
{
  CodedInputStream coded_stream(input_stream);

  std::uint32_t number;
  if(!coded_stream.ReadLittleEndian32(&number)) {
    throw std::runtime_error("Could not read magic number.");
  }

  if(number != magic_number) {
    throw std::runtime_error("Magic numbers did not match.");
  }
}

bool istream::read(google::protobuf::Message *message)
{
  CodedInputStream coded_stream(input_stream);
  uint32_t size;

  if(coded_stream.ReadVarint32(&size)) {
    auto const limit = coded_stream.PushLimit(static_cast<int>(size));

    if(message->ParseFromCodedStream(&coded_stream)) {
      coded_stream.PopLimit(limit);

      return true; // there are more messages.
    } else {
      throw std::runtime_error("Unable to read message from protobuf file.");
    }
  }

  return false; // EOF
}
} // namespace ioproto