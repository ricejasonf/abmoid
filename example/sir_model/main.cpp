#include <abmoid/agent.hpp>
#include <abmoid/agent_component.hpp>
#include <abmoid/rk4.hpp>

#include <iostream>
#include <random>
#include <ranges>
#include <utility>

struct parameters {
  double gamma;
  double beta;
  unsigned N;
  unsigned I_0;
  unsigned contact_factor;
};

struct susceptible_state {
  // Frames until infected;
  unsigned timer = 0;
};
struct recovered_state { };

struct infected_state {
  // Frames until recovered.
  unsigned timer;
};

class agent_sir_model {
  using id_type = abmoid::agent::id_type;
  using seed_type = std::mt19937::result_type;

  double gamma;
  double beta;
  double beta_star;
  abmoid::population N;
  unsigned contact_factor;
  std::mt19937 gen;
  abmoid::agent_component<susceptible_state> S;
  abmoid::agent_component<infected_state> I;
  abmoid::agent_component<recovered_state> R;

  void init(unsigned I_0) {
    for (abmoid::agent agent : std::views::take(N, I_0))
      assign_I(agent);
    
    for (abmoid::agent agent : std::views::drop(N, I_0))
      S.create(agent, susceptible_state{0});
  }

  double gen_uniform_random() {
    return std::uniform_real_distribution<double>()(gen);
  }

  void assign_I(abmoid::agent a) {
    double rand = std::exponential_distribution<>(gamma)(gen);
    unsigned initial_timer = static_cast<unsigned>(std::round(rand));

    I.create(a, infected_state{initial_timer});
  }

  bool is_valid() const {
    // TODO Check that the intersection
    //      of sets of agents in each set is empty.
    // The SIR states are mutually exclusive.
    return N.size() == S.size() + I.size() + R.size();
  }

  void update_S() {
    // Iterate susceptibles and possibly make sick.
    for (auto itr = S.begin(); itr != S.end();) {
      susceptible_state s = *itr;
      if (s.timer == 1) {
        // Create random duration based on exponential distribution.
        assign_I(S.get_agent(itr));
        itr = S.erase(itr);
        continue;
      } else if (s.timer > 1) {
        --s.timer;
      } else {
        // Roll the dice and maybe get contact.
        double I_over_N = static_cast<double>(I.size()) /
                          static_cast<double>(N.size());
        if (gen_uniform_random() < I_over_N) {
          double rand = std::exponential_distribution<>(beta_star)(gen);
          s.timer = static_cast<unsigned>(std::round(rand));
          if (s.timer == 0) {
            assign_I(S.get_agent(itr));
            itr = S.erase(itr);
            continue;
          }
        }
      }

      ++itr;
    }
  }

  void update_I() {
    // Iterate infecteds updating timer and possibly make recovered.
    for (auto itr = I.begin(); itr != I.end();) {
      infected_state& state = *itr;
      if (state.timer == 0) {
        abmoid::agent agent = I.get_agent(itr);
        R.create(agent);
        I.erase(itr);
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
  agent_sir_model(parameters params,
        seed_type seed = std::mt19937::default_seed)
    : gamma(params.gamma),
      beta(params.beta),
      beta_star(params.beta * params.contact_factor),
      contact_factor(params.contact_factor),
      gen(seed),
      N(params.N)
  {
    init(params.I_0);
  }

  void reset(unsigned I_0) {
    S.clear();
    I.clear();
    R.clear();
    init(I_0);
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
};

struct ode_sir_model {
  struct state {
    double S;
    double I;
    double R;

    state operator+(state const& other) const {
      return state{.S = S + other.S,
                   .I = I + other.I,
                   .R = R + other.R};
    }

  };

  double N;     // Total population
  double beta;  // Transmission rate (per day)
  double gamma; // Recovery rate (per 1/day)

  state operator()(double t, state vec) const {
    auto [S, I, R] = vec;
    return state{
      -beta * I * S / N,
      beta * I * S / N - gamma * I,
      gamma * I
    };
  }
};

ode_sir_model::state operator*(double k, ode_sir_model::state const& self) {
  return {.S = k * self.S,
          .I = k * self.I,
          .R = k * self.R};
}

ode_sir_model::state operator*(ode_sir_model::state const& self, double k) {
  return k * self;
}

template <typename HandleFn>
void run_sir_agent(unsigned seed, parameters params, unsigned total_frames,
                   HandleFn handle) {
  agent_sir_model sir(params, seed);

  // Simulate stuff.
  for (unsigned i = 0; i < total_frames; ++i) {
    sir.update();
    auto [S, I, R] = sir.get_state();
    handle(S, I, R, i);
  }
}

template <typename HandleFn>
void run_sir_ode(parameters params, unsigned total_frames, HandleFn handle) {
  double N    = static_cast<double>(params.N);
  double I_0  = static_cast<double>(params.I_0);

  ode_sir_model sir{.N      = static_cast<double>(N),
                    .beta   = params.beta,
                    .gamma  = params.gamma};
  ode_sir_model::state init_state{
    .S = N - I_0,
    .I = I_0,
    .R = 0.0};

  auto step_result = [&](ode_sir_model::state state, abmoid::time_t t) {
    auto [S, I, R] = state;
    handle(S, I, R, t);
  };

  // Calculate stuff.
  abmoid::rk4(sir, step_result, init_state,
              abmoid::time_step{0.01}, total_frames * 100);
}

int main() {
  int const total_frames = 364;
  parameters params{.gamma  = 0.10,
                    .beta   = 0.24,
                    .N      = 10'000,
                    .I_0    = 10,
                    .contact_factor = 2};

  auto print_csv_row = [](unsigned S, unsigned I, unsigned R, unsigned t) {
    std::cout << S << ',' << I << ',' << R << '\n';
  };

  auto begin_new_dataset = [] { std::cout << "\n\n"; };

  run_sir_ode(params, total_frames, print_csv_row);

  for (int i = 0; i < 100; i++) {
    begin_new_dataset();
    run_sir_agent(i, params, total_frames, print_csv_row);
  }
}
