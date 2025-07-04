#ifndef SIR_SOCIAL_HPP
#define SIR_SOCIAL_HPP

#include <abmoid/agent.hpp>
#include <abmoid/agent_component.hpp>

#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace sir_social {
struct group_params {
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
  std::vector<group_params> groups;
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

// Cache counts and store simulation data needed
// for each social group when updating susceptible
// population.
struct group_state {
  unsigned I_count = 0;
  unsigned N_count = 0;
  double beta_star;

  group_state(double beta_star)
    : beta_star(beta_star)
  { }
};

struct group_name {
  std::string_view value;
};

// Track social group connections and relevant
// simulation data.
class social_group_connections {

  abmoid::agent_component<group_name, social_group> group_names;
  abmoid::agent_component<group_state, social_group> groups;
  std::unordered_set<std::pair<social_group, person>> connections;
  std::unordered_map<std::string_view, social_group> name_lookup;

  group_state& get_group_state_helper(social_group g) {
    auto group_itr = groups.find(g);
    assert(group_itr != groups.end());
    return *group_itr;
  }

public:
  social_group_connections() = default;

  auto const& get_group_names() const { return group_names; }
  auto const& get_group_states() const { return groups; }

  group_state const& get_group_state(social_group g) const {
    return const_cast<social_group_connections&>(*this)
      .get_group_state_helper(g);
  }

  void init_group(social_group g, group_params const& params) {
    name_lookup[params.name] = g;
    double beta_star = params.beta * params.contact_factor;
    group_names.create(g, group_name(params.name));
    groups.create(g, group_state(beta_star));
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
    group_state& group = get_group_state_helper(g);

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
        group_state& group = *itr;
        if (is_infected)
          ++group.I_count;
        else
          --group.I_count;
      }
    }
  }
};

class agent_model {
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
        for (unsigned i = 0; i < conn_spec.N; ++i) {
          person p = people.push_back();
          S.create(p, susceptible_state{0});
          for (std::string_view group_name : conn_spec.groups)
            connections.add(p, group_name, /*is_infected=*/false);
        }
        for (unsigned i = 0; i < conn_spec.I_0; ++i) {
          person p = people.push_back();
          assign_I(p);
          for (std::string_view group_name : conn_spec.groups)
            connections.add(p, group_name, /*is_infected=*/true);
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
        // TODO Possibly handle agent counts in intersection of groups.
        for (social_group g : social_groups) {
          if (connections.contains(g, S.get_agent(itr))) {
            auto [I_g, N_g, beta_star_g] = connections.get_group_state(g);
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

  agent_model(parameters const& params,
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

  // Return const range of group_name.
  auto const& get_group_names() const {
    return connections.get_group_names();
  }

  // Return const range of group_state.
  auto const& get_group_states() const {
    return connections.get_group_states();
  }
};
}

#endif
