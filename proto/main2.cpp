#include <abmoid/agent.hpp>
#include <abmoid/agent_component.hpp>

#include <matplot/matplot.h>
#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

struct susceptible_state { };
struct recovered_state { };

struct infected_state {
  // Frames until recovered.
  unsigned timer;
};

class agent_sir_model {
  using id_type = abmoid::agent::id_type;

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
      S.create(agent);
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
      // Choose 3 randos and check for infectedness.
      bool is_infected = false;
      for (unsigned i = 0; i < contact_factor; ++i) {
        abmoid::agent e = N.select_random(gen);
        if (I.contains(e) && gen_uniform_random() < beta_star) {
          is_infected = true;
          break;
        }
      }
      if (is_infected) {
        // Create random duration based on exponential distribution.
        assign_I(S.get_agent(itr));
        itr = S.erase(itr);
      } else {
          ++itr;
      }
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
  struct parameters {
    double gamma;
    double beta;
    unsigned N;
    unsigned I_0;
    unsigned contact_factor;
  };

  agent_sir_model(parameters params)
    : gamma(params.gamma),
      beta(params.beta),
      beta_star(params.beta * params.contact_factor),
      contact_factor(params.contact_factor),
      gen(),
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

int main() {
  std::mt19937 gen;
  int const total_frames = 364;
  std::vector<double> S_counts(total_frames, 0.0);
  std::vector<double> I_counts(total_frames, 0.0);
  std::vector<double> R_counts(total_frames, 0.0);

  agent_sir_model sir({.gamma = 0.10,
                       .beta = 0.24,
                       .N = 10'000,
                       .I_0 = 10,
                       .contact_factor = 1});

  // Simulate stuff.
  for (unsigned i = 0; i < total_frames; ++i) {
    sir.update();
    auto [S, I, R] = sir.get_state();
    S_counts[i] = static_cast<double>(S);
    I_counts[i] = static_cast<double>(I);
    R_counts[i] = static_cast<double>(R);
  }

  // Plot stuff.
  auto figure = matplot::figure(true);
  matplot::title("Agent Based SIR Model");
  matplot::legend(matplot::on);
  matplot::hold(matplot::on);


  auto plot1 = matplot::plot(S_counts);
  plot1->line_width(2);
  plot1->display_name("S");

  auto plot2 = matplot::plot(I_counts);
  plot2->line_width(2);
  plot2->display_name("I");

  auto plot3 = matplot::plot(R_counts);
  plot3->line_width(2);
  plot3->display_name("R");

  matplot::save("img/b_plot.png");
  matplot::hold(matplot::off);
}
