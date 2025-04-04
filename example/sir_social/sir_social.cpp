#include <iostream>

#include "sir_social.hpp"

int main() {
  using parameters = sir_social::parameters;
  using group_params = sir_social::group_params;
  using connection_spec = sir_social::connection_spec;

  auto params_1 = parameters{
    .gamma  = 0.10,
    .groups{
      group_params{
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
        .N = 6'665,
        .I_0 = 0
      },
      connection_spec{
       .groups = {"B"},
       .N = 3'310,
       .I_0 = 20
      },
      connection_spec{
       .groups = {"A", "B"},
       .N = 5,
       .I_0 = 0
      },
    }
  };

  int const total_frames = 364;

  // Printing functions
  auto print_csv_row = [](unsigned S, unsigned I, unsigned R, unsigned t) {
    std::cout << S << ',' << I << ',' << R << '\n';
  };
  auto begin_new_dataset = [] { std::cout << "\n\n"; };

  {
    // Simulate single group.
    sir_social::agent_model sir(params_1);
    for (unsigned i = 0; i < total_frames; ++i) {
      sir.update();
      auto [S, I, R] = sir.get_state();
      print_csv_row(S, I, R, i);
    }
  }

  begin_new_dataset();

  {
    // Simulate two groups.
    sir_social::agent_model sir(params_2);
    for (unsigned i = 0; i < total_frames; ++i) {
      sir.update();
      auto [S, I, R] = sir.get_state();
      print_csv_row(S, I, R, i);
    }
  }
}
