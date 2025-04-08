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

// Br - number bridge connections
//     (ie cardinality of intersection of both groups)
// Return peak times for each group.
peak_result run_model(unsigned Br, unsigned A_size, unsigned B_size) {
  using parameters = sir_social::parameters;
  using group_params = sir_social::group_params;
  using connection_spec = sir_social::connection_spec;

  assert(A_size <= 10'000);
  unsigned I_0 = 5;
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
        .N = A_size,
        .I_0 = I_0
      },
      connection_spec{
       .groups = {"B"},
       .N = 10'000 - A_size,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"A", "B"},
       .N = Br,
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
  // Br - Number of bridge connections
  constexpr unsigned Br_max = 100;
  constexpr unsigned Br_step = 5;

  // A - The group "A" where the infection starts.
  constexpr unsigned A_size_max = 9'500;
  constexpr unsigned A_size_step = 250;

  for (unsigned A_size = 9'500; A_size > 0; A_size -= A_size_step) {
    unsigned B_size = 10'000 - A_size;
    for (unsigned Br = 1; Br <= Br_max; Br += Br_step) {
      peak_result peaks = run_model(Br, A_size, B_size);
      // Each group's peak time is a column in the csv output
      // along with Br.
      std::cout << Br << ',';
      std::cout << A_size << ',';
      std::cout << B_size << ',';
      std::cout << peaks[0].t << ',';
      std::cout << peaks[1].t << '\n';
    }
    std::cout << "\n\n";  // Begin new dataset.
  }
}
