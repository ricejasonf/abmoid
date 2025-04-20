#include <array>
#include <fstream>

#include "sir_social.hpp"

int main() {
  constexpr unsigned total_frames = 364;
  constexpr double beta = 0.24;
  unsigned I_0 = 20;

  auto group = [](std::string_view name) {
    return sir_social::group_params{
      .name= name,
      .beta = beta,
      .contact_factor = 2
    };
  };

  using connection_spec = sir_social::connection_spec;

  auto params = sir_social::parameters{
    .gamma  = 0.10,
    .groups{
      group("A"),
      group("B"),
      group("C"),
      group("D"),
      group("E")
    },
    .connections{
      // Define group populations.
      connection_spec{
        .groups = {"A"},
        .N = 5'050 - I_0,
        .I_0 = I_0
      },
      connection_spec{
       .groups = {"B"},
       .N = 5'000,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"C"},
       .N = 5'000,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"D"},
       .N = 5'050,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"E"},
       .N = 5'000,
       .I_0 = 0
      },
      // Connect the groups.
      connection_spec{
       .groups = {"A", "B"},
       .N = 30,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"B", "C"},
       .N = 30,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"C", "D"},
       .N = 30,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"D", "E"},
       .N = 30,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"B", "E"},
       .N = 30,
       .I_0 = 0
      }
    }
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
