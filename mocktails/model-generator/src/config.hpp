#ifndef MOCKTAILS_CONFIG_HPP
#define MOCKTAILS_CONFIG_HPP

#include <string>

#include "mocktails/profile.hpp"
#include "mocktails/hierarchy.hpp"

namespace mocktails {

model_type parse_model_type(std::string const &type);

/**
 * Parse the file to create a configuration of a mocktails hierarchy.
 */
hierarchy::configuration parse_configuration(std::string const &config_filename);
}

#endif //MOCKTAILS_CONFIG_HPP
