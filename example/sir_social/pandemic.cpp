#include <array>
#include <fstream>

#include "sir_social.hpp"

int main() {
  constexpr unsigned total_frames = 364;
  constexpr double beta = 0.24;
  // Scale the already scaled input population data.
  // (1 / 2)^{sci_scale_factor}
  unsigned sci_scale_factor = 4;

  struct entry {
    std::string_view code_from;
    std::string_view code_to;
    unsigned sci_value;
    unsigned I_0;  // not scaled
  };

  std::vector<entry> input_data{
#include<country_connections.hpp>
  };

  auto group = [](std::string_view name) {
    return sir_social::group_params{
      .name= name,
      .beta = beta,
      .contact_factor = 2
    };
  };

  using group_params = sir_social::group_params;
  using connection_spec = sir_social::connection_spec;

  std::vector<group_params> groups;
  std::vector<connection_spec> connections;
  std::vector<std::string_view> group_names;
  for (entry const& x : input_data) {
    // Scale and round the SCI values and ignore zeros.
    unsigned sci_value = entry.sci_value >> sci_scale_factor;

    // If the codes are the same, that is just the population.
    group_names.clear();
    group_names.push_back(x.code_from);
    if (x.code_from == x.code_to)
      group_names.push_back(x.code_to);

    connections.push_back(connection_spec{
      .groups = group_names
      .N = sci_value,
      .I_0 = I_0
    });
  }

  auto params = sir_social::parameters{
    .gamma  = 0.10,
    .groups = groups,
    .connections = connections,
  };
  sir_social::agent_model sir(params);

  auto infected_data = std::ofstream("data/sir_network_infected.dat");
  //auto graph_data = std::ofstream("data/sir_network_graph.dat");

  // Print population datasets consisting of row with group names
  // and a row for the total populations for each group.
  infected_data << "# t, ";
  for (auto [name] : sir.get_group_names())
    infected_data << name << "_N, ";
  infected_data << "\n";

  // Plot total population for each group.
  infected_data << "# Group population counts.\n";
  infected_data << "0";
  for (auto [I_count, N_count, beta_star] : sir.get_group_states())
    infected_data << ", " << N_count;
  infected_data << "\n\n\n";

  infected_data << "# Group infected counts.\n";
  for (unsigned t = 0; t < total_frames; ++t) {
    sir.update();
    infected_data << t;
    for (auto const& group : sir.get_group_states())
      infected_data << ", " << group.I_count;
    infected_data << '\n';
  }
}
