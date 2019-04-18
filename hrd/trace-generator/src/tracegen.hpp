#ifndef HRD_CLONING_TRACEGEN_HPP
#define HRD_CLONING_TRACEGEN_HPP

#include <string>

/**
 * Create a synthetic trace based on the HRD statistical profile.
 *
 * @param input_filename The path to the statistical profile.
 * @param output_filename The trace file to serialize to.
 * @param request_count The number of requests to generate, or 0 to match the profile.
 */
void generate_trace(std::string const &input_filename, std::string const &output_filename);

#endif //HRD_CLONING_TRACEGEN_HPP
