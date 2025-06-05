#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "sir_social.hpp"

struct connection_entry {
  std::string_view code_from;
  std::string_view code_to;
  unsigned sci_value;
  unsigned I_0;  // not scaled
};

struct national_population_entry {
  std::string_view code;
  std::string_view aux_code;
  std::string_view name;
  unsigned population;
};

static std::vector<connection_entry> input_country_connections{
#include "country_connections.hpp"
};

static std::unordered_map<std::string_view, national_population_entry>
input_national_pops{
#include "national_pops.hpp"
};

unsigned get_national_pop(std::string_view code) {
  auto itr = input_national_pops.find(code);
  if (itr == input_national_pops.end())
    return 0;
  return itr->second.population;
}


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
  for (connection_entry const& x : input_country_connections) {
    // Remove duplicates by lexicographical comparison.
    if (x.code_from > x.code_to)
      continue;

    // Do not include countries with small populations.
    unsigned pop = get_national_pop(x.code_from);
    unsigned pop_to = get_national_pop(x.code_to);
    constexpr unsigned min_pop = 50'000'000;
    if (pop < min_pop || pop_to < min_pop)
      continue;

    // Scale and round the SCI values.
    unsigned sci_value = 0;
    if (x.code_from == x.code_to) {
      // Create entry for country population
      // (in place of connections to self.)
      sci_value = pop >> (sci_scale_factor + 1);
    } else {
      sci_value = x.sci_value >> sci_scale_factor;
    }

    auto nation_itr = input_national_pops.find(x.code_from);
#if 0
    if (x.code_from == x.code_to && nation_itr != input_national_pops.end()) {
      auto nation = nation_itr->second;
      std::cout << nation.name << " (I_0 = " << x.I_0 << "): " <<
                   nation.population << " -> " << sci_value << '\n';
    }
#endif

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
  unsigned sci_scale_factor = 11;
  std::vector<group_params> groups;
  std::vector<connection_spec> connections;

#if 0
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
#endif
  generate_inputs(sci_scale_factor, groups, connections);

  std::cout << "\nBegin simulation!\n";

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
  std::cout << "t = 000";
  std::cout.flush();
  for (unsigned t = 0; t < total_frames; ++t) {
    sir.update();
    infected_data << t;
    for (auto const& group : sir.get_group_states())
      infected_data << ", " << group.I_count;
    infected_data << '\n';
    std::cout << "\b\b\b";  // Erase the previous time digits.
    std::cout << std::setfill('0') << std::setw(3) << t;
    std::cout.flush();
  }
  std::cout << '\n';

  // Output max infected count.
}
