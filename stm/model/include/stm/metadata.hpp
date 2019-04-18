#ifndef STM_CLONING_METADATA_HPP
#define STM_CLONING_METADATA_HPP

#include <memory>

#include "ioproto/istream.hpp"
#include "ioproto/ofstream.hpp"

#include "stm/profile.hpp"

namespace stm {

/**
 * Read a profile from the input stream.
 *
 * @return nullptr if nothing could be read from the stream.
 */
std::unique_ptr<profile> read(ioproto::istream &stream);

/**
 * Append a profile to the output stream.
 */
void append(ioproto::ofstream &stream, profile const &p);

} // namespace stm

#endif //STM_CLONING_METADATA_HPP
