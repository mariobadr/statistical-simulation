#ifndef HRD_CLONING_METADATA_HPP
#define HRD_CLONING_METADATA_HPP

#include <memory>

#include <ioproto/istream.hpp>
#include <ioproto/ofstream.hpp>

#include <hrd/profile.hpp>

namespace hrd {

/**
 * Read a profile from the input stream.
 */
profile read(ioproto::istream &stream, std::uint64_t *request_count = nullptr);

/**
 * Append a profile to the output stream.
 */
void append(ioproto::ofstream &stream, profile const &p);
}

#endif //HRD_CLONING_METADATA_HPP
