#ifndef IOPROTO_OSTREAM_HPP
#define IOPROTO_OSTREAM_HPP

#include <fstream>
#include <memory>

#include <google/protobuf/message.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace ioproto {

/**
 * Write size-delimited protobuf messages to a file.
 */
class ofstream {
public:
  /**
   * Constructor.
   *
   * @param file_name A path to the output file.
   */
  explicit ofstream(std::string const &file_name);

  /**
   * Recommended constructor.
   *
   * Writes a magic number to the file, as recommended by the protobuf documentation.
   *
   * @param file_name A path to the output file.
   * @param magic_number The number to write.
   */
  ofstream(std::string const &file_name, std::uint32_t magic_number);

  /**
   * Write a protobuf message to the file.
   *
   * @param message The message to serialize.
   */
  void write(google::protobuf::Message const &message);

private:
  std::ofstream standard_stream;

  std::unique_ptr<google::protobuf::io::OstreamOutputStream> wrapped_fstream = nullptr;
  std::unique_ptr<google::protobuf::io::GzipOutputStream> gzip_stream = nullptr;
  google::protobuf::io::ZeroCopyOutputStream *output_stream = nullptr;
};
} // namespace ioproto

#endif //IOPROTO_OSTREAM_HPP
