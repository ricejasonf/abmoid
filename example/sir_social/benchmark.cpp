#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <ratio>
#include <string_view>
#include <utility>

#include "sir_social.hpp"

static std::array<std::string_view, 10> group_names{
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
};

// Return pair(agent_model, num_agents).
std::optional<std::pair<sir_social::agent_model, /*num_agents :*/unsigned>>
create_simulation(unsigned group_size, unsigned num_groups, unsigned I_0) {
  if (num_groups > group_names.size())
    return {};

  // Keep a tally of agents we add.
  unsigned num_agents = 0;

  constexpr unsigned total_frames = 364;
  constexpr double beta = 0.24;

  auto group = [](std::string_view name) {
    return sir_social::group_params{
      .name= name,
      .beta = beta,
      .contact_factor = 2
    };
  };

  using connection_spec = sir_social::connection_spec;

  std::vector<sir_social::group_params> groups;
  for (unsigned i = 0; i < num_groups; i++)
    groups.push_back(sir_social::group_params(group_names[i]));

  std::vector<sir_social::connection_spec> connections;
  auto add_conn = [&num_agents, &connections]
                  (sir_social::connection_spec const& c) {
                    num_agents += c.N;
                    num_agents += c.I_0;
                    connections.push_back(c);
                  };

  // Add agents exclusive to each group.
  for (unsigned i = 0; i < num_groups; i++) {
    unsigned group_I_0 = (i == 0) ?  I_0 : 0;
    auto conn_spec = sir_social::connection_spec{
      .groups = {group_names[i]},
      .N = group_size,
      .I_0 = group_I_0};
    add_conn(conn_spec);
  }

  // Add agents connections between adjacent groups.
  if (num_groups > 0)
    for (unsigned i = 0; i < (num_groups - 1); i++) {
      unsigned I_0 = (i == 0) ?  20 : 0;
      auto conn_spec = sir_social::connection_spec{
        .groups = {group_names[i], group_names[i + 1]},
        .N = 100,
        .I_0 = 0};
      add_conn(conn_spec);
    }

  auto params = sir_social::parameters{
    .gamma  = 0.10,
    .groups{std::move(groups)},
    .connections{std::move(connections)}};
  sir_social::agent_model sir(params);

  return std::make_pair(std::move(sir), num_agents); 
}

int main() {
  constexpr unsigned max_group_count = group_names.size();
  auto benchmark_dat = std::ofstream("data/benchmark.dat");
  using clock = std::chrono::steady_clock;
  using time_point = std::chrono::time_point<clock>;
  using duration_t = std::chrono::duration<double, std::milli>;
  using time_ms = std::chrono::milliseconds;
  static_assert(clock::is_steady,
                "clock should be monotonicly increasing and nonadjustable");

  auto run_sim = [](sir_social::agent_model& sir) -> time_ms {
    constexpr unsigned total_frames = 364;
    time_point start = clock::now();
    for (unsigned t = 0; t < total_frames; ++t)
      sir.update();
    time_point end = clock::now();
    return time_ms(std::chrono::duration_cast<time_ms>(end - start));
  };

  // Benchmark a single group of various sizes.
  for (unsigned i = 0; i <= 200'000; i += 500) {
    duration_t mean_elapsed;
    unsigned num_agents = 0;
    unsigned num_groups = 1;
    unsigned group_size = i;
    unsigned I_0 = 20; // Note we are not scaling I_0 with group_size.
    for (unsigned n = 1; n <= 20; n++) {
      auto result = create_simulation(group_size, num_groups, I_0);
      if (!result)
        std::cerr << "Similation error!\n";
      auto [sir, num_agents_] = *result;
      num_agents = num_agents_;
      time_ms elapsed = run_sim(sir);
      mean_elapsed += (elapsed - mean_elapsed) / n;
    }
    benchmark_dat << num_groups << ", "
                  << group_size << ", "
                  << num_agents << ", "
                  << mean_elapsed << '\n';
  }

  // Create new data set.
  benchmark_dat << "\n\n";

  // Benchmark a single group of various sizes.
  for (unsigned i = 0; i <= 10; i++) {
    duration_t mean_elapsed;;
    unsigned num_agents = 0;
    unsigned num_groups = i;
    unsigned group_size = 5'000;
    unsigned I_0 = 20;
    for (unsigned n = 1; n <= 20; n++) {
      auto result = create_simulation(group_size, num_groups, I_0);
      if (!result)
        std::cerr << "Similation error!\n";
      auto [sir, num_agents_] = *result;
      num_agents = num_agents_;
      time_ms elapsed = run_sim(sir);
      mean_elapsed += (elapsed - mean_elapsed) / n;
    }
    benchmark_dat << num_groups << ", " << num_agents << ", " << mean_elapsed << '\n';
  }

}
