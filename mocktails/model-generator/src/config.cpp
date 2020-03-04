#include "config.hpp"

#include <fstream>

#include "json.hpp"

namespace mocktails {

using nlohmann::json;

bool is_configured(json const &config, std::string const &key)
{
  return config.count(key) != 0;
}

partition::type parse_partition_type(std::string const &partition_type)
{
  static const std::map<std::string, partition::type> mapping = {
      {"spatial", partition::type::spatial},
      {"temporal", partition::type::temporal},
  };

  auto const it = mapping.find(partition_type);
  if(it == mapping.end()) {
    throw std::runtime_error("Could not find partition type: " + partition_type);
  }

  return it->second;
}

partition::delimiter parse_partition_separator(std::string const &partition_separator)
{
  static const std::map<std::string, partition::delimiter> mapping = {
      {"contiguous", partition::delimiter::contiguous}, {"bytes", partition::delimiter::bytes},
      {"cycles", partition::delimiter::cycles}, {"count", partition::delimiter::count},
      {"requests", partition::delimiter::requests}};

  auto const it = mapping.find(partition_separator);
  if(it == mapping.end()) {
    throw std::runtime_error("Could not find partition separator: " + partition_separator);
  }

  return it->second;
}

partition::configuration parse_partition_configuration(json const &json_config)
{
  if(!is_configured(json_config, "partition")) {
    throw std::runtime_error("Invalid configuration.");
  }

  auto const &partition_config = json_config["partition"];
  partition::configuration config;

  if(!is_configured(partition_config, "scheme")) {
    throw std::runtime_error("Partition 'scheme' was not configured.");
  } else {
    config.scheme = parse_partition_type(partition_config["scheme"]);
  }

  if(!is_configured(partition_config, "separator")) {
    throw std::runtime_error("Partition 'separator' was not configured.");
  } else {
    config.separator = parse_partition_separator(partition_config["separator"]);
  }

  if(!is_configured(partition_config, "value")) {
    throw std::runtime_error("Partition 'value' was not configured.");
  } else {
    config.separator_value = partition_config["value"];
  }

  if(config.separator_value <= 0) {
    throw std::runtime_error("Partition 'value' must be greater than 0");
  }

  return config;
}

model_type parse_model_type(std::string const &type)
{
   static const std::map<std::string, model_type> mapping = {
      {"mocktails", model_type::mocktails},
      {"stm", model_type::stm},
      {"hrd", model_type::hrd}
  };

  auto const it = mapping.find(type);
  if(it == mapping.end()) {
    throw std::runtime_error("Could not find model type: " + type);
  }

  return it->second;
}

hierarchy::configuration parse_configuration(std::string const &config_filename)
{
  std::ifstream config_file(config_filename);
  json json_config = json::parse(config_file);

  if(!is_configured(json_config, "hierarchy")) {
    throw std::runtime_error("A hierarchy was not configured.");
  }

  hierarchy::configuration config;

  // Add the default monolithic configuration for root.
  config.levels.emplace_back();

  for(auto const &json_level : json_config["hierarchy"]) {
    config.levels.emplace_back(parse_partition_configuration(json_level));
  }

  return config;
}

} // namespace mocktails