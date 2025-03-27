#include <abmoid/agent.hpp>
#include <abmoid/agent_component.hpp>

#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

// Simulation wide params
struct parameters {
  double gamma;
  unsigned N;
};

struct social_group_params {
  std::string name; // not currently used
  double beta;
  unsigned N;
  unsigned I_0;
  unsigned contact_factor;
};

// People as agents
struct person_tag { };
using person = abmoid::agent_t<person_tag>;

// Social groups as agents
struct social_group_tag { };
using social_group = abmoid::agent_t<social_group_tag>;

struct susceptible_state {
  // Frames until infected;
  unsigned timer = 0;
};
struct recovered_state { };

struct infected_state {
  // Frames until recovered.
  unsigned timer;
};

// Track social group connections and relevant
// simulation data.
class social_group_connections {
  // Cache counts and store simulation data needed
  // for each social group when updating susceptible
  // population.
  struct group_data {
    unsigned I_count = 0;
    unsigned N_count = 0;
    double beta_star;

    group_data(double beta_star)
      : beta_star(beta_star)
    { }
  };

  abmoid::agent_component<group_data, social_group> groups;
  std::unordered_set<std::pair<person, social_group>> connections;

public;
  void init_group(social_group g, social_group_params const& params) {
    double beta_star = params.beta * param.contact_factor;
    groups.create(g, group_data(beta_star));
  }

  void add(person p, social_group g, bool is_infected) {
    // Add an entry to the set.
    auto [itr, did_insert] = connections.insert({p, g});
    assert(did_insert && "should add connection only once");

    // TODO lookup component by agent.
    group_data& group = groups.get_agent(g);

    // Increment N_count if connection did not exist.
    ++group.N_count;

    // Increment infected_count if infected.
    if (is_infected)
      ++group.I_count;
  }

  // For a person changing infected state, update
  // the groups counts for each group.
  void update(person p, bool is_infected) {
    for (auto itr = groups.begin(); itr != groups.end();) {

    // TODO
  }
};

class agent_sir_model {
  using id_type = abmoid::agent::id_type;

  double gamma;
  abmoid::population N;
  std::mt19937 gen;
  abmoid::agent_component<susceptible_state> S;
  abmoid::agent_component<infected_state> I;
  abmoid::agent_component<recovered_state> R;
  std::array<social_group_component, 2> social_groups;


  template <typename SocialGroupParams>
  void init(SocialGroupParams&& sg_params) {
    // Create the social group components.
    for (auto& sg : social_groups) {
      social_groups.push_back(social_group_component{
        .infected_count = sg_params.infected_count,
        .beta           = sg_params.beta,
        .contact_factor = sg_params.contact_factor});
    }

    // Track agents that are unassigned to SIR state.
    auto remaining_pop = std::views::drop(N, 0);

    // Add the infecteds to the social groups.
    for (social_group_component& sg : social_groups) {
      for (abmoid::agent agent : std::views::take(remaining_pop,
                                                  sg.infected_count)) {
        assign_I(agent);
        assign_to_group(sg, agent);
      }
      remaining_pop = std::views::drop(remaining_pop, sg.infected_count);
    }

    // Assign all that remains the susceptible state.
    for (abmoid::agent agent : remaining_pop)
      S.create(agent);

    // Calculate intersection
    // 
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
      susceptible_state s = *itr;
      if (s.timer == 1) {
        assign_I(S.get_agent(itr));
        itr = S.erase(itr);
        continue;
      } else if (s.timer > 1) {
        --s.timer;
      } else {
        for (social_group& g : social_groups) {
          if (connections.contains({s, g})) {
            social_group_sim_data group = connections.group_data.get_agent(g);
            double I_over_N = static_cast<double>(group.I) /
                              static_cast<double>(group.N);
    
            if (gen_uniform_random() < I_over_N) {
              double rand =
                std::exponential_distribution<>(group.beta_star)(gen);
              s.timer = static_cast<unsigned>(std::round(rand));
              if (s.timer == 0) {
                assign_I(S.get_agent(itr));
                itr = S.erase(itr);
                break;
              }
            }
          }
        }
      }
      ++itr;
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
      N(params.N),
      social_groups(std::from_range, social_group_params)
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
