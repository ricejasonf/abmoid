#ifndef PEAK_TIMES_HPP
#define PEAK_TIMES_HPP

#include <algorithm>
#include <array>
#include <future>
#include <iostream>

#include "sir_social.hpp"

namespace peak_times {
constexpr unsigned group_count = 2;

struct peak {
  unsigned I;
  unsigned t;
};
using peak_result = std::array<peak, group_count>;

// Br - number bridge connections
//     (ie cardinality of intersection of both groups)
// Return peak times for each group.
peak_result run_model(unsigned Br, unsigned A_N, unsigned B_N,
                      unsigned seed) {
  using parameters = sir_social::parameters;
  using group_params = sir_social::group_params;
  using connection_spec = sir_social::connection_spec;

  assert(A_N <= 10'000);
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
        .N = A_N,
        .I_0 = I_0
      },
      connection_spec{
       .groups = {"B"},
       .N = 10'000 - A_N,
       .I_0 = 0
      },
      connection_spec{
       .groups = {"A", "B"},
       .N = Br,
       .I_0 = 0
      },
    }
  };

  sir_social::agent_model sir(params, seed);

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

struct result_row {
  unsigned Br;
  unsigned A_N;
  unsigned B_N;
  unsigned A_I_max_t;
  unsigned B_I_max_t;
};

using result_set = std::vector<result_row>;

void print_result_row(result_row const& row) {
  auto const& [Br, A_N, B_N, A_I_max_t, B_I_max_t] = row;
  std::cout << Br << ',' <<
               A_N << ',' <<
               B_N << ',' <<
               A_I_max_t << ',' <<
               B_I_max_t << '\n';
}

void print_result_set(result_set const& rows) {
  for (result_row const& row : rows)
    return print_result_row(row);
  std::cout << "\n\n";
}

template <typename RunExperimentFn, typename HandleResultSetFn>
void run_experiments(unsigned total_runs, unsigned max_threads,
                     RunExperimentFn&& run,
                     HandleResultSetFn&& handle_result) {
  std::vector<std::future<result_set>> result_sets;
  std::atomic<unsigned> jobs_finished = 0;

  while (total_runs > 0) {
    unsigned num_runs = std::min(total_runs, max_threads);

    // Start up a bunch of asynchronous runs.
    for (unsigned i = 0; i < max_threads; i++)
      result_sets.push_back(std::async(std::launch::async, run));

    // For each run, block this thread until it gets the result,
    // and then handle it.
    for (auto& future : result_sets) {
      handle_result(future.get());
    }

    total_runs -= num_runs;
  }
}

}

#endif
