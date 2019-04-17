#include "ioproto/ofstream.hpp"

#include <google/protobuf/io/coded_stream.h>

namespace ioproto {

using google::protobuf::Message;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::io::GzipOutputStream;
using google::protobuf::io::OstreamOutputStream;
using google::protobuf::io::ZeroCopyOutputStream;

bool is_gzipped(std::string const &filename)
{
  auto const extension = filename.find_last_of('.');
  if(extension == std::string::npos) {
    return false;
  }

  return filename.substr(extension + 1) == "gz";
}

ofstream::ofstream(std::string const &file_name)
    : standard_stream(file_name, std::ios::out | std::ios::binary | std::ios::trunc)
{
  wrapped_fstream = std::make_unique<OstreamOutputStream>(&standard_stream);
  output_stream = wrapped_fstream.get();

  // Use gzip if it is present in the filename.
  if(is_gzipped(file_name)) {
    gzip_stream = std::make_unique<GzipOutputStream>(wrapped_fstream.get());
    output_stream = gzip_stream.get();
  }
}

ofstream::ofstream(std::string const &file_name, std::uint32_t magic_number) : ofstream(file_name)
{
  CodedOutputStream coded_stream(output_stream);
  coded_stream.WriteLittleEndian32(magic_number);
}

void ofstream::write(google::protobuf::Message const &message)
{
  // Determine the size of the message in bytes.
  auto const size = static_cast<std::uint32_t>(message.ByteSize());

  // Create the stream for each message (because reasons).
  CodedOutputStream coded_stream(output_stream);
  // Write the size of the message.
  coded_stream.WriteVarint32(size);
  // Write the message itself.
  message.SerializeWithCachedSizes(&coded_stream);
}

} // namespace ioproto