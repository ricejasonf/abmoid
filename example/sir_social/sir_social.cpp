#include <abmoid/agent.hpp>
#include <abmoid/agent_component.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <random>
#include <ranges>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

struct social_group_params {
  std::string_view name;
  double beta; // rate of infection
  unsigned contact_factor;
};

struct connection_spec {
  std::vector<std::string_view> groups;
  unsigned N;
  unsigned I_0;
};

struct parameters {
  double gamma; // rate recovery
  std::vector<social_group_params> groups;
  std::vector<connection_spec> connections;
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
  std::unordered_set<std::pair<social_group, person>> connections;
  std::unordered_map<std::string_view, social_group> name_lookup;

  group_data& get_group_data_helper(social_group g) {
    auto group_itr = groups.find(g);
    assert(group_itr != groups.end());
    return *group_itr;
  }

public:
  social_group_connections() = default;

  group_data const& get_group_data(social_group g) const {
    return const_cast<social_group_connections&>(*this)
      .get_group_data_helper(g);
  }

  void init_group(social_group g, social_group_params const& params) {
    name_lookup[params.name] = g;
    double beta_star = params.beta * params.contact_factor;
    groups.create(g, group_data(beta_star));
  }

  social_group get(std::string_view name) const {
    auto itr = name_lookup.find(name);
    if (itr != name_lookup.end())
      return itr->second;
    
    // Return invalid.
    return {};
  }

  bool contains(social_group g, person p) const {
    return connections.contains({g, p});
  }

  void add(person p, social_group g, bool is_infected) {
    // Add an entry to the set.
    auto [itr, did_insert] = connections.insert({g, p});
    assert(did_insert && "should add connection only once");

    // Get the group data associated with the group agent.
    group_data& group = get_group_data_helper(g);

    // Increment N_count if connection did not exist.
    ++group.N_count;

    // Increment infected_count if infected.
    if (is_infected)
      ++group.I_count;
  }

  void add(person p, std::string_view group_name, bool is_infected) {
    return add(p, get(group_name), is_infected);
  }

  // For a person changing infected state, update
  // the groups counts for each group.
  // We assume `is_infected` is not the same as
  // the current state.
  void update(person p, bool is_infected) {
    for (auto itr = groups.begin(); itr != groups.end(); ++itr) {
      social_group g = groups.get_agent(itr);
      if (connections.contains({g, p})) {
        group_data& group = *itr;
        if (is_infected)
          ++group.I_count;
        else
          --group.I_count;
      }
    }
  }
};

class agent_sir_model {
  double gamma;
  abmoid::population_t<person> people;
  abmoid::population_t<social_group> social_groups;
  std::mt19937 gen;
  abmoid::agent_component<susceptible_state, person> S;
  abmoid::agent_component<infected_state, person> I;
  abmoid::agent_component<recovered_state, person> R;
  social_group_connections connections;

  void init(parameters const& params) {
    S.clear();
    I.clear();
    R.clear();

    auto pairs = std::ranges::views::zip(social_groups, params.groups);
    for (auto const& [g, params] : pairs)
      connections.init_group(g, params);

    for (connection_spec const& conn_spec : params.connections) {
        for (unsigned i = 0; i < conn_spec.N; ++i)
          for (std::string_view group_name : conn_spec.groups) {
            person p = people.push_back();
            connections.add(p, group_name, /*is_infected=*/false);
            S.create(p, susceptible_state{0});
          }
        for (unsigned i = 0; i < conn_spec.I_0; ++i)
          for (std::string_view group_name : conn_spec.groups) {
            person p = people.push_back();
            connections.add(p, group_name, /*is_infected=*/true);
            assign_I(p);
          }
    }
  }

  double gen_uniform_random() {
    return std::uniform_real_distribution<double>()(gen);
  }

  void assign_I(person a) {
    double rand = std::exponential_distribution<>(gamma)(gen);
    unsigned initial_timer = static_cast<unsigned>(std::round(rand));

    I.create(a, infected_state{initial_timer});
    connections.update(a, /*is_infected=*/true);
  }

  void update_S() {
    // Iterate susceptibles and possibly make sick.
    for (auto itr = S.begin(); itr != S.end();) {
      // Count the number of nodes connected to this susceptible person.
      unsigned total_count = 0;
      unsigned infected_count = 0;
      auto infected_itr = S.end();
      susceptible_state s = *itr;
      if (s.timer == 1) {
        infected_itr = itr;
      } else if (s.timer > 1) {
        --s.timer;
      } else {
        for (social_group g : social_groups) {
          if (connections.contains(g, S.get_agent(itr))) {
            auto [I_g, N_g, beta_star_g] = connections.get_group_data(g);
            double I_over_N = static_cast<double>(I_g) /
                              static_cast<double>(N_g);

            if (gen_uniform_random() < I_over_N) {
              double rand =
                std::exponential_distribution<>(beta_star_g)(gen);
              s.timer = static_cast<unsigned>(std::round(rand));
              if (s.timer == 0) {
                infected_itr = itr;
                break;
              }
            }
          }
        }
      }

      if (infected_itr != S.end()) {
        assign_I(S.get_agent(itr));
        itr = S.erase(itr);
      } else
        ++itr;
    }
  }

  void update_I() {
    // Iterate infecteds updating timer and possibly make recovered.
    for (auto itr = I.begin(); itr != I.end();) {
      infected_state& state = *itr;
      if (state.timer == 0) {
        person p = I.get_agent(itr);
        R.create(p);
        I.erase(itr);
        connections.update(p, /*is_infected=*/false);
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
  using seed_type = std::mt19937::result_type;

  agent_sir_model(parameters const& params,
        seed_type seed = std::mt19937::default_seed)
    : gamma(params.gamma),
      gen(seed),
      people(0),
      social_groups(params.groups.size())
  {
    init(params);
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

int main() {
  auto params_1 = parameters{
    .gamma  = 0.10,
    .groups{
      social_group_params{
        .name= "A",
        .beta = 0.24,
        .contact_factor = 2
      }
    },
    .connections{
      connection_spec{
        .groups = {"A"},
        .N = 9'090,
        .I_0 = 10
      }
    }
  };

  auto params_2 = parameters{
    .gamma  = 0.10,
    .groups{
      social_group_params{
        .name= "A",
        .beta = 0.24,
        .contact_factor = 2
      },
      social_group_params{
        .name= "B",
        .beta = 0.24,
        .contact_factor = 2
      }
    },
    .connections{
      connection_spec{
        .groups = {"A"},
        .N = 7'490,
        .I_0 = 10
      },
      connection_spec{
       .groups = {"B"},
       .N = 2'490,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"A", "B"},
       .N = 10,
       .I_0 = 0
      },
    }
  };

  // TODO count connections.

  int const total_frames = 364;

  // Printing functions
  auto print_csv_row = [](unsigned S, unsigned I, unsigned R, unsigned t) {
    std::cout << S << ',' << I << ',' << R << '\n';
  };
  auto begin_new_dataset = [] { std::cout << "\n\n"; };

  {
    // Simulate stuff.
    agent_sir_model sir(params_1);
    for (unsigned i = 0; i < total_frames; ++i) {
      sir.update();
      auto [S, I, R] = sir.get_state();
      print_csv_row(S, I, R, i);
    }
  }

  begin_new_dataset();

  {
    // Simulate stuff again.
    agent_sir_model sir(params_2);
    for (unsigned i = 0; i < total_frames; ++i) {
      sir.update();
      auto [S, I, R] = sir.get_state();
      print_csv_row(S, I, R, i);
    }
  }
}
