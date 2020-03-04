#include "mocktails/synthesis.hpp"

#include <stm/synthesis.hpp>
#include <hrd/synthesis.hpp>

namespace mocktails {

template <typename Type>
bool converge(Type &value)
{
  assert(value > 0);
  auto new_value = value - 1;

  if(new_value > value) {
    value = 0;
  } else {
    value = new_value;
  }

  return value == 0;
}

std::uint64_t keep_in_range(std::uint64_t address, std::uint64_t min, std::uint64_t max)
{
  if(address > max || address < min) {
    const auto size = max - min;
    address = ((address - size) % size) + min;
  }

  return address;
}

template <typename Type>
Type generate_next(std::mt19937 &rng, sequence_model<Type> &sm)
{
  if(sm.transitions.empty()) {
    return sm.initial_state;
  }

  auto current_it = sm.transitions.find(sm.current_state);
  if(current_it == sm.transitions.end()) {
    // The current state is not in the Markov chain, select a new current state.

    std::vector<Type> states;
    std::vector<std::uint64_t> counts;

    for(auto const &pair : sm.transitions) {
      states.emplace_back(pair.first);

      std::uint64_t count = 0;
      for(auto const &pair2 : pair.second) {
        count += pair2.second;
      }
      counts.emplace_back(count);
    }

    std::discrete_distribution<std::size_t> distribution(counts.begin(), counts.end());
    auto const index = distribution(rng);
    sm.current_state = states[index];
  }

  std::vector<Type> states;
  std::vector<std::uint64_t> counts;
  for(auto const &pair : sm.transitions[sm.current_state]) {
    states.push_back(pair.first);
    counts.push_back(pair.second);
  }

  std::discrete_distribution<std::size_t> distribution(counts.begin(), counts.end());
  auto const index = distribution(rng);
  auto const next_state = states[index];

  if(converge(sm.transitions[sm.current_state][next_state])) {
    sm.transitions[sm.current_state].erase(next_state);
  }

  if(sm.transitions[sm.current_state].empty()) {
    sm.transitions.erase(sm.current_state);
  }

  sm.current_state = next_state;

  return next_state;
}

void populate_queue(std::mt19937 &rng,
    std::priority_queue<request> &queue,
    model<simple_model> &model)
{
  auto running_time = model.start_time;
  auto current_address = model.underlying_model->start_address;
  auto size = generate_next(rng, model.size_model);
  auto op = generate_next(rng, model.underlying_model->operation_model);

  queue.emplace(running_time, op, current_address, size);

  for(std::size_t i = 1; i < model.request_count; i++) {
    auto const delta_time = generate_next(rng, model.time_model);
    running_time += delta_time;

    auto const stride = generate_next(rng, model.underlying_model->stride_model);
    current_address += stride;

    current_address = keep_in_range(current_address, model.underlying_model->footprint.start,
        model.underlying_model->footprint.end);

    size = generate_next(rng, model.size_model);
    op = generate_next(rng, model.underlying_model->operation_model);

    queue.emplace(running_time, op, current_address, size);
  }
}

mocktails::operation convert(stm::operation op)
{
  if(op == stm::operation::read) {
    return mocktails::operation::read;
  } else {
    return mocktails::operation::write;
  }
}

void populate_queue(std::mt19937 &rng,
    std::priority_queue<request> &queue,
    model<stm::profile> &model)
{
  auto request_count = model.underlying_model->count();

  auto running_time = model.start_time;
  auto size = generate_next(rng, model.size_model);
  std::uint64_t address;

  stm::synthesiser stm_synthesiser(*model.underlying_model);
  stm::operation stm_operation;
  stm_synthesiser.generate_next_request(address, stm_operation);

  auto op = convert(stm_operation);
  queue.emplace(running_time, op, address, size);


  for(std::size_t i = 1; i < request_count; i++) {
    auto const delta_time = generate_next(rng, model.time_model);
    running_time += delta_time;

    size = generate_next(rng, model.size_model);
    stm_synthesiser.generate_next_request(address, stm_operation);

    queue.emplace(running_time, convert(stm_operation), address, size);
  }
}

mocktails::operation convert(hrd::operation op)
{
  if(op == hrd::operation::read) {
    return mocktails::operation::read;
  } else {
    return mocktails::operation::write;
  }
}

void populate_queue(std::mt19937 &rng,
    std::priority_queue<request> &queue,
    model<hrd::profile> &model)
{
  auto running_time = model.start_time;
  auto size = generate_next(rng, model.size_model);
  std::uint64_t address;

  hrd::synthesiser hrd_synthesiser(*model.underlying_model);
  hrd::operation hrd_operation;
  hrd_synthesiser.generate_next_request(address, hrd_operation);

  auto op = convert(hrd_operation);
  queue.emplace(running_time, op, address, size);

  for(std::size_t i = 1; i < model.request_count; i++) {
    auto const delta_time = generate_next(rng, model.time_model);
    running_time += delta_time;

    size = generate_next(rng, model.size_model);
    hrd_synthesiser.generate_next_request(address, hrd_operation);

    queue.emplace(running_time, op, address, size);
  }
}

synthesiser::synthesiser(profile &p)
{
  if(p.type == model_type::mocktails) {
    for(auto &leaf : p.simple_leaves) {
      populate_queue(m_rng, requests, leaf.second);
    }
  } else if(p.type == model_type::stm) {
    for(auto &leaf : p.stm_leaves) {
      populate_queue(m_rng, requests, leaf.second);
    }
  } else if(p.type == model_type::hrd) {
    for(auto &leaf : p.hrd_leaves) {
      populate_queue(m_rng, requests, leaf.second);
    }
  }
}

bool synthesiser::has_more_requests() const
{
  return !requests.empty();
}

request synthesiser::generate_next_request()
{
  auto next_request = requests.top();
  requests.pop();

  return next_request;
}

} // namespace mocktails