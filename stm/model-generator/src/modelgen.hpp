#ifndef STM_CLONING_MODELGEN_HPP
#define STM_CLONING_MODELGEN_HPP

#include <string>

#include "stm/profile.hpp"

void generate_stm_model(std::string const &input_filename,
    std::string const &output_filename,
    stm::profile::parameters const &parameters,
    std::uint64_t interval_size);

#endif //STM_CLONING_MODELGEN_HPP
