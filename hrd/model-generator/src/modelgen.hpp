#ifndef HRD_CLONING_MODELGEN_HPP
#define HRD_CLONING_MODELGEN_HPP

#include <string>
#include <vector>

#include "hrd/profile.hpp"

/**
 * Generate and serialize an HRD statistical profile.
 *
 * @param input_filename The trace file to read memory requests from.
 * @param output_filename The model file to serialize to.
 * @param layers The block sizes to use in the hierarchy.
 */
void generate_hrd_model(std::string const &input_filename,
    std::string const &output_filename,
    std::vector<std::uint64_t> layers);

#endif //HRD_CLONING_MODELGEN_HPP
