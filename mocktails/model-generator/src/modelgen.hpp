#ifndef MOCKTAILS_MODELGEN_HPP
#define MOCKTAILS_MODELGEN_HPP

#include <string>

namespace mocktails {

/**
 * Generate a mocktails model.
 *
 * @param input_filename A valid path to the trace file.
 * @param output_filename A path to the output file.
 * @param config_filename A valid path to the configuration file.
 * @param type The type of model to use for the leaves.
 * @param root_size The maximum number of requests for each hierarchy.
 */
void generate_model(std::string const &input_filename,
    std::string const &output_filename,
    std::string const &config_filename,
    std::string const &type,
    std::uint64_t root_size);
} // namespace mocktails

#endif //MOCKTAILS_MODELGEN_HPP
