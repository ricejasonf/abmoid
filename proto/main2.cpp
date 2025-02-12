#include <matplot/matplot.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <random>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

namespace abmoid {

struct agent {
  using id_type = uint_fast32_t;
  id_type id;

  bool is_valid() const {
    return id != 0;
  }

  bool operator==(agent const&) const = default;
};

}

template <>
struct std::hash<abmoid::agent> {
  uint32_t operator()(abmoid::agent agent) const noexcept {
    return std::hash<abmoid::agent::id_type>{}(agent.id);
  }
};

namespace abmoid {

template <typename Value>
class agent_component {
  using value_storage = std::vector<Value>;
  using agent_storage = std::vector<agent>;
  using lookup_storage = std::unordered_map<agent, unsigned>;
  using index_t = unsigned;

  value_storage values;
  agent_storage agents;
  lookup_storage lookup;

public:
  using iterator = value_storage::iterator;

  void clear() {
    values.clear();
    agents.clear();
    lookup.clear();
  }

  auto size() const { return values.size(); }
  iterator begin() const { return values.begin(); }
  iterator begin() { return values.begin(); }
  iterator end() const { return values.end(); }
  iterator end() { return values.end(); }
  iterator erase(iterator itr) {
    index_t index = std::distance(values.begin(), itr);
    auto agent_itr = agents.begin() + index;
    auto lookup_itr = lookup.find(get_agent(itr));
    assert(agents.size() > index &&
        "corresponding agent entry should exist");
    assert(lookup_itr != lookup.end() &&
        "component must exist to erase it");

    if (size() == 1) {
      clear();
      return end();
    }

    lookup.erase(lookup_itr);

    // Remove the value and agent entries by
    // swapping with the last element so we can efficiently
    // remove the element without reindexing everything.
    using std::swap;
    swap(*itr, values.back());
    swap(*agent_itr, agents.back());
    lookup[*agent_itr] = index;
    values.pop_back();
    agents.pop_back();

    // Since itr was swapped with the back, we do not increment.
    return itr;
  }

  bool contains(agent a) {
    return lookup.contains(a);
  }

  template <typename V>
  Value& create(agent a, V&& value) {
    assert(!contains(a) && "only one component per entity is allowed");
    lookup[a] = values.size();
    values.push_back(std::forward<V>(value));
    agents.push_back(a);
    return values.back();
  }

  agent get_agent(index_t index) const {
    assert(index < agents.size());
    return agents[index];
  }

  agent get_agent(iterator itr) {
    return get_agent(std::distance(values.begin(), itr));
  }
};

template <typename DistImpl>
class distribution : std::ranges::view_interface<distribution<DistImpl>> {
  std::mt19937& gen;
  DistImpl dist;

public:
  template <typename ...Args>
  distribution(std::mt19937& gen, Args&& ...args)
    : gen(gen),
      dist(std::forward<Args>(args)...)
  { }

  class iterator {
    using Range = distribution;

    Range* range;
    DistImpl::result_type value;

    DistImpl::result_type get_next() {
      return range->dist(range->gen);
    }

    iterator(Range* range)
      : range(range),
        value(get_next())
    { }

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = DistImpl::result_type;

    value_type operator*() const {
      return value;
    }

    iterator& operator++() {
      value = get_next();
      return *this;
    }

    void operator++(int) {
      ++*this;
    }
  };

  iterator begin() const {
    return iterator(this);
  }
  auto end() const {
    return std::unreachable_sentinel;
  }
};

using weibull_dist = distribution<std::weibull_distribution<>>;
using exponential_dist = distribution<std::exponential_distribution<>>;

static_assert(std::input_iterator<typename weibull_dist::iterator>);
static_assert(std::input_iterator<typename exponential_dist::iterator>);
}  // namespace abmoid

struct susceptible_state { };
struct recovered_state { };

struct infected_state {
  // Frames until recovered.
  unsigned timer;
};

class agent_sir_model {
  using id_type = abmoid::agent::id_type;

  double gamma;
  double beta;
  double beta_star;
  double recover_mean;
  unsigned N; // Population size
  unsigned contact_factor;
  std::mt19937 gen;
  abmoid::agent_component<susceptible_state> susceptible;
  abmoid::agent_component<infected_state> infected;
  abmoid::agent_component<recovered_state> recovered;

  void init(unsigned initial_infecteds) {
    // Each agent is denoted implicitly by an id that
    // is in the range [1, N].
    id_type i = 1;
    for (; i <= initial_infecteds; ++i)
      assign_infected_state(abmoid::agent{i});
    for (; i <= N; ++i)
      susceptible.create(abmoid::agent{i}, susceptible_state{});
  }

  double gen_uniform_random() {
    return std::uniform_real_distribution<double>()(gen);
  }

  // Select a random agent from the population.
  abmoid::agent select_random_agent() {
    return abmoid::agent{std::uniform_int_distribution<id_type>(1, N)(gen)};
  }

  void assign_infected_state(abmoid::agent a) {
    // double rand = std::exponential_distribution<>(recover_mean)(gen);

    // infected.create(a, infected_state{static_cast<unsigned>(rand * 145.0)});
    // Every day 10% of infecteds heal. Somehow a timer of 7 does the job.
    infected.create(a, infected_state{static_cast<unsigned>(7)});
  }

  bool is_valid() const {
    // TODO Check that the intersection
    //      of sets of agents in each set is empty.
    // The SIR states are mutually exclusive.
    return N == susceptible.size() +
                infected.size() +
                recovered.size();
  }

  void update_susceptible() {
    // Iterate susceptibles and possibly make sick.
    for (auto itr = susceptible.begin(); itr != susceptible.end();) {
      // Choose 3 randos and check for infectedness.
      bool is_infected = false;
      for (unsigned i = 0; i < contact_factor; ++i) {
        abmoid::agent e = select_random_agent();
        if (infected.contains(e) && gen_uniform_random() < beta_star) {
          is_infected = true;
          break;
        }
      }
      if (is_infected) {
        // Create random duration based on exponential distribution.
        assign_infected_state(susceptible.get_agent(itr));
        itr = susceptible.erase(itr);
      } else {
          ++itr;
      }
    }
  }

  void update_infected() {
    // Iterate infecteds updating timer and possibly make recovered.
    for (auto itr = infected.begin(); itr != infected.end();) {
      infected_state& state = *itr;
      if (state.timer == 0) {
        abmoid::agent agent = infected.get_agent(itr);
        recovered.create(agent, recovered_state{});
        infected.erase(itr);
      } else {
        state.timer -= 1;
        ++itr;
      }
    }
  }

  void update_recovered() {
    // Do nothing.
  }

public:
  struct parameters {
    double gamma;
    double beta;
    unsigned N;
    unsigned initial_infecteds;
    unsigned contact_factor;
  };

  agent_sir_model(parameters params)
    : gamma(params.gamma),
      beta(params.beta),
      beta_star(params.beta / params.contact_factor),
      recover_mean(1.0 / params.gamma), // ??
      contact_factor(params.contact_factor),
      gen(),
      N(params.N)
  {
    init(params.initial_infecteds);
  }

  void reset(unsigned initial_infecteds) {
    susceptible.clear();
    infected.clear();
    recovered.clear();
    init(initial_infecteds);
  }

  // Each frame we call update.
  void update() {
    update_susceptible();
    update_infected();
    update_recovered();
  }

  auto get_state() const {
    return std::array<size_t, 3>{{
      susceptible.size(),
      infected.size(),
      recovered.size()
    }};
  }
};

int main() {
  std::mt19937 gen;
  int const total_frames = 364;
  std::vector<double> S_counts(total_frames, 0.0);
  std::vector<double> I_counts(total_frames, 0.0);
  std::vector<double> R_counts(total_frames, 0.0);

  agent_sir_model sir({.gamma = 0.10,
                       .beta = 0.24,
                       .N = 1000,
                       .initial_infecteds = 1,
                       .contact_factor = 3});

  // Simulate stuff.
  for (unsigned i = 0; i < total_frames; ++i) {
    sir.update();
    auto [S, I, R] = sir.get_state();
    S_counts[i] = static_cast<double>(S);
    I_counts[i] = static_cast<double>(I);
    R_counts[i] = static_cast<double>(R);
  }

  // Plot stuff.
  auto figure = matplot::figure(true);
  matplot::title("Agent Based SIR Model");
  matplot::legend(matplot::on);
  matplot::hold(matplot::on);


  auto plot1 = matplot::plot(S_counts);
  plot1->line_width(2);
  plot1->display_name("S");

  auto plot2 = matplot::plot(I_counts);
  plot2->line_width(2);
  plot2->display_name("I");

  auto plot3 = matplot::plot(R_counts);
  plot3->line_width(2);
  plot3->display_name("R");

  matplot::save("img/b_plot.png");
  matplot::hold(matplot::off);
}
