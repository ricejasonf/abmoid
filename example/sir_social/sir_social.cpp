#include <abmoid/agent.hpp>
#include <abmoid/agent_component.hpp>

#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

struct parameters {
  double gamma;
  unsigned N;
};

struct social_group {
  std::string name;
  double beta;
  unsigned N;
  unsigned I_0;
  unsigned contact_factor;
};

struct susceptible_state { };
struct recovered_state { };
struct social_group_connection { };

struct infected_state {
  // Frames until recovered.
  unsigned timer;
};

struct social_group_component {
  unsigned infected_count;
  // Rate of infection
  double beta;
  // Number of adjacent agents within this group that
  // we randomly inspect for infection
  unsigned contact_factor;
  abmoid::agent_component component<social_group_connection>;
};

class agent_sir_model {
  using id_type = abmoid::agent::id_type;

  double gamma;
  double beta;
  double beta_star;
  abmoid::population N;
  unsigned contact_factor;
  std::mt19937 gen;
  abmoid::agent_component<susceptible_state> S;
  abmoid::agent_component<infected_state> I;
  abmoid::agent_component<recovered_state> R;
  std::array<social_group_component, 2> social_groups;

  void init(unsigned I_0) {
    for (abmoid::agent agent : std::views::take(N, I_0))
      assign_I(agent);
    for (abmoid::agent agent : std::views::drop(N, I_0))
      S.create(agent);
  }

  double gen_uniform_random() {
    return std::uniform_real_distribution<double>()(gen);
  }

  void assign_I(abmoid::agent a) {
    double rand = std::exponential_distribution<>(gamma)(gen);
    unsigned initial_timer = static_cast<unsigned>(std::round(rand));

    I.create(a, infected_state{initial_timer});
  }

  bool is_valid() const {
    // TODO Check that the intersection
    //      of sets of agents in each set is empty.
    // The SIR states are mutually exclusive.
    return N.size() == S.size() + I.size() + R.size();
  }

  void update_S() {
    // Count the infecteds in each social group up front.
    for (abmoid::agent infected : infecteds)
      for (social_group_component& group : social_groups)
        if (group.component.contains(infected)
          ++group.infected_count;

    // Iterate susceptibles and possibly make sick.
    for (auto itr = S.begin(); itr != S.end();) {
      // Count the number of nodes connected to this susceptible agent.
      unsigned total_count = 0;
      unsigned infected_count = 0;
      bool is_infected = false;
      for (social_group_component& group : social_groups) {
        if (group.contains(*itr)) {
          double probability = group.beta *
                               static_cast<double>(group.infected_count) /
                               static_cast<double>(group.component.size());
          if (gen_uniform_random() < probability) {
            is_infected = true;
            break;
          }
        }
      }
      if (is_infected) {
        // Create random duration based on exponential distribution.
        assign_I(S.get_agent(itr));
        itr = S.erase(itr);
      } else {
          ++itr;
      }
    }
  }

  void update_I() {
    // Iterate infecteds updating timer and possibly make recovered.
    for (auto itr = I.begin(); itr != I.end();) {
      infected_state& state = *itr;
      if (state.timer == 0) {
        abmoid::agent agent = I.get_agent(itr);
        R.create(agent);
        I.erase(itr);
      } else {
        state.timer -= 1;
        ++itr;
      }
    }
  }

  void update_R() {
    // Do nothing.
  }

public:
  agent_sir_model(parameters params,
                  std::initializer_list<social_group> social_group_params)
    : gamma(params.gamma),
      gen(),
      N(params.N)
  {
    for (social_group&& s : social_group_params)
      init_social_group(s);
  }

  void reset(unsigned I_0) {
    S.clear();
    I.clear();
    R.clear();
    init(I_0);
  }

  // Each frame we call update.
  void update() {
    update_S();
    update_I();
    update_R();
  }

  auto get_state() const {
    return std::array<size_t, 3>{{S.size(), I.size(), R.size()}};
  }
};

void main() {
  agent_sir_model sir(
    parameters{
      .gamma  = 0.10,
      .N      = 10'000
    },
    {
      social_group{
        .name = "A",
        .N = 5'010,
        .beta = 0.24,
        .I_0 = 10,
        .contact_factor = 3
      },
      social_group{
       .name = "B",
       .N = 5'000,
       .beta = 0.24,
       .I_0 = 0,
       .contact_factor = 3
      }
    });

  int const total_frames = 364;

  // Simulate stuff.
  for (unsigned i = 0; i < total_frames; ++i) {
    sir.update();
    auto [S, I, R] = sir.get_state();
    S_counts[i] = static_cast<double>(S);
    I_counts[i] = static_cast<double>(I);
    R_counts[i] = static_cast<double>(R);
    time_vals[i] = static_cast<double>(i);
  }
}
