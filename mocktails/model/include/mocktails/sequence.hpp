#ifndef MOCKTAILS_SEQUENCE_MODEL_HPP
#define MOCKTAILS_SEQUENCE_MODEL_HPP

#include <cassert>
#include <set>

namespace mocktails {

/**
 * Maintain a transition matrix as nested associative containers that track the frequency of
 * transitions.
 */
template <typename Type>
using transition_matrix = std::map<Type, std::map<Type, std::uint64_t>>;

/**
 * A sequence can be modelled as a constant value (i.e., the sequence never changes) or
 * as a transition_matrix for a Markov chain.
 */
template <typename Type>
struct sequence_model {
  Type initial_state = Type();
  Type current_state = Type();
  transition_matrix<Type> transitions;
};

template <typename Type>
transition_matrix<Type> create_transition_matrix(std::vector<Type> const &sequence)
{
  assert(sequence.size() > 1);

  transition_matrix<Type> tm;
  auto last_it = sequence.begin();
  for(auto it = std::next(last_it); it != sequence.end(); ++it) {
    tm[*last_it][*it]++;

    last_it = it;
  }

  return tm;
}

template <typename Type>
class sequence {
public:
  void add(Type v)
  {
    trace.emplace_back(v);
    set.insert(v);
  }

  bool is_constant() const
  {
    return set.size() == 1;
  }

  sequence_model<Type> make_model() const
  {
    sequence_model<Type> model{};

    if(trace.empty()) {
      return model;
    }

    if(!is_constant()) {
      model.transitions = create_transition_matrix(trace);
    }

    model.initial_state = trace.front();

    return model;
  }

private:
  std::vector<Type> trace;
  std::set<Type> set;
};

} // namespace mocktails

#endif //MOCKTAILS_SEQUENCE_MODEL_HPP
