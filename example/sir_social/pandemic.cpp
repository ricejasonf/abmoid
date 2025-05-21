#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>

#include "sir_social.hpp"

struct entry {
  std::string_view code_from;
  std::string_view code_to;
  unsigned sci_value;
  unsigned I_0;  // not scaled
};

static std::vector<entry> input_country_connections{
#include "country_connections.hpp"
};

static std::vector<entry> input_national_pops{
#include "national_pops.hpp"
};


using group_params = sir_social::group_params;
using connection_spec = sir_social::connection_spec;

void generate_inputs(unsigned sci_scale_factor,
                     std::vector<group_params>& groups,
                     std::vector<connection_spec>& connections) {
  constexpr double beta = 0.24;
  groups.clear();
  connections.clear();

  auto group = [](std::string_view name) {
    return sir_social::group_params{
      .name= name,
      .beta = beta,
      .contact_factor = 2
    };
  };

  std::vector<std::string_view> group_names;
  for (entry const& x : input_data) {
    // Remove duplicates by lexicographical comparison.
    if (x.code_from > x.code_to)
      continue;

    // Scale and round the SCI values.
    unsigned sci_value = x.sci_value >> sci_scale_factor;

    // Cull insignificant sci_values.
    if (sci_value == 0)
      continue;

    // If the codes are the same, that is just the population.
    group_names.clear();
    group_names.push_back(x.code_from);
    if (x.code_from == x.code_to)
      groups.push_back(group(x.code_from));
    else
      group_names.push_back(x.code_to);

    connections.push_back(connection_spec{
      .groups = group_names,
      .N = sci_value,
      .I_0 = x.I_0
    });
  }

  // Remove groups with no connections.
  // (Because their population sci_value was large enough
  //  but no connections were.)
  for (auto itr = groups.begin(); itr != groups.end();) {
    std::string_view name = itr->name;
    size_t count = std::count_if(connections.begin(), connections.end(),
      [&](connection_spec const& conn) {
        return (conn.groups.size() == 2 &&
                (conn.groups[0] == name ||
                 conn.groups[1] == name));
      });

    if (count == 0) {
      itr = groups.erase(itr);
      // Remove the solitary connection node.
      std::erase_if(connections,
        [&](connection_spec const& conn) {
          return conn.groups.size() > 0 &&
                 conn.groups[0] == name;
        });
    } else {
      ++itr;
    }
  }
}

int main() {
  constexpr unsigned total_frames = 364;
  // Scale the already scaled input population data.
  // (1 / 2)^{sci_scale_factor}
  unsigned sci_scale_factor = 32;
  std::vector<group_params> groups;
  std::vector<connection_spec> connections;

  while (true) {
    std::cout << "\nPlease enter the sci_scale_factor: ";
    if (!(std::cin >> sci_scale_factor)) {
      std::cout << "\nInput error!\n";
      std::exit(1);
    }
    generate_inputs(sci_scale_factor, groups, connections);
    std::cout << "There are:"
                 "\n\tCountries:\t" << groups.size() <<
                 "\n\tConnections:\t" << connections.size() <<
                 "\n\nDo you wish to proceed with simulation? [yes/no] ";

    std::string answer;
    while ((std::cin >> answer) && answer != "yes" && answer != "no") {
      // continue
    }

    if (answer == "yes")
      break;
  }

  std::cout << "\nBegin simulation!\n";

#if 0
  auto params = sir_social::parameters{
    .gamma  = 0.10,
    .groups = groups,
    .connections = connections,
  };
  sir_social::agent_model sir(params);

  auto infected_data = std::ofstream("data/pandemic.dat");

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

  // Output max infected count.
#endif
}
