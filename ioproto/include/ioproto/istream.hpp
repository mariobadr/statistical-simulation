#ifndef IOPROTO_ISTREAM_HPP
#define IOPROTO_ISTREAM_HPP

#include <cstdint>
#include <iosfwd>
#include <memory>

#include <google/protobuf/message.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>

namespace ioproto {

/**
 * Read size-delimited protobuf messages from an input stream.
 */
class istream {
public:
  /**
   * Constructor.
   *
   * @param stream The input stream to read from.
   */
  explicit istream(std::istream &stream);

  /**
   * Constructor.
   *
   * @param stream The input stream to read from.
   * @param magic_number The expected magic number.
   *
   * @throw std::runtime_error if the expected magic number was not found.
   */
  istream(std::istream &stream, std::uint32_t magic_number);

  /**
   * Read a message from the input stream.
   *
   * @param message The data from the input stream will be put into this message.
   *
   * @return true if there are more messages, false if the input stream has reached EOF.
   *
   * @throw std::runtime_error if the function failed to read from the input stream.
   */
  bool read(google::protobuf::Message *message);

private:
  std::unique_ptr<google::protobuf::io::ZeroCopyInputStream> parent_stream;
  std::unique_ptr<google::protobuf::io::GzipInputStream> gzip_stream;

  google::protobuf::io::ZeroCopyInputStream *input_stream;
};

} // namespace ioproto

#endif //IOPROTO_ISTREAM_HPP
