#ifndef MOCKTAILS_SIMPLE_MODEL_HPP
#define MOCKTAILS_SIMPLE_MODEL_HPP

#include <map>
#include <memory>
#include <vector>

#include <hrd/profile.hpp>
#include <stm/profile.hpp>

#include <mocktails/request.hpp>
#include <mocktails/sequence.hpp>

namespace mocktails {

/**
 * A wrapper that models common characteristics, and includes an underlying model for other characteristics.
 *
 * @tparam ModelType The type of the underlying model.
 */
template <typename ModelType>
struct model {
  /**
   * The number of requests modelled.
   */
  std::uint64_t request_count = 0;

  /**
   * The time of the first request.
   */
  std::uint64_t start_time = 0;

  /**
   * Models the number of bytes being requested.
   */
  sequence_model<std::uint64_t> size_model;

  /**
   * Models the deltas between generated time stamps.
   */
  sequence_model<std::uint64_t> time_model;

  /**
   * Models additional characteristics (e.g., stride, operations).
   */
  std::unique_ptr<ModelType> underlying_model = nullptr;
};

/**
 * A simple underlying model type.
 */
struct simple_model {
  /**
   * The address of the first request.
   */
  address_t start_address = 0;

  /**
   * The address range modelled.
   */
  address_range footprint;

  /**
   * Models whether the request is a read or a write.
   */
  sequence_model<operation> operation_model;

  /**
   * Models the deltas between generated addresses.
   */
  sequence_model<stride_t> stride_model;
};

model<simple_model> create_simple_model(std::vector<request> const &requests);

model<stm::profile> create_stm_model(std::vector<request> const &requests);

model<hrd::profile> create_hrd_model(std::vector<request> const &requests);

} // namespace mocktails

#endif //MOCKTAILS_SIMPLE_MODEL_HPP
