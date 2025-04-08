#include <array>
#include <iostream>
#include <vector>

#include "sir_social.hpp"

constexpr unsigned group_count = 2;

struct peak {
  unsigned I;
  unsigned t;
};
using peak_result = std::array<peak, group_count>;

// B - number bridge connections
//     (ie cardinality of intersection of both groups)
// Return peak times for each group.
peak_result run_model(unsigned B) {
  using parameters = sir_social::parameters;
  using group_params = sir_social::group_params;
  using connection_spec = sir_social::connection_spec;

  unsigned I_0 = 20;
  constexpr unsigned total_frames = 364;

  // Number of agents should be 10'000.
  auto params = parameters{
    .gamma  = 0.10,
    .groups{
      group_params{
        .name= "A",
        .beta = 0.24,
        .contact_factor = 2
      },
      group_params{
        .name= "B",
        .beta = 0.24,
        .contact_factor = 2
      }
    },
    .connections{
      connection_spec{
        .groups = {"A"},
        .N = 7'500 - I_0 - (B / 2 + B % 2),
        .I_0 = I_0
      },
      connection_spec{
       .groups = {"B"},
       .N = 2'500 - (B / 2),
       .I_0 = 0
      },
      connection_spec{
       .groups = {"A", "B"},
       .N = B,
       .I_0 = 0
      },
    }
  };

  sir_social::agent_model sir(params);

  // Merely take the maximum I_t value for the peak
  // for each group. This assumes a single peak or at least
  // ignores lesser peaks.
  assert(group_count == sir.get_group_states().size());

  struct peak { unsigned I, t; };
  peak_result peaks{};

  for (unsigned t = 0; t < total_frames; ++t) {
    sir.update();

    auto pairs = std::ranges::views::zip(sir.get_group_states(), peaks);
    for (auto&& pair : pairs) {
      auto&& [state, peak] = pair;
      auto const& [I, N, beta_star] = state;
      if (I > peak.I)
        peak = {I, t};
    }
  }

  return peaks;
}

int main() {
  constexpr unsigned run_count = 100;
  std::vector<unsigned> peak_times(run_count * group_count);
  for (unsigned i = 0; i < run_count; i++) {
    unsigned B = i + 1;
    peak_result peaks = run_model(B);
    // Each group's peak time is a column in the csv output.
    std::cout << peaks[0].t << ',';
    std::cout << peaks[1].t << '\n';
  }
}
