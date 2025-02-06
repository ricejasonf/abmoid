#include <matplot/matplot.h>

#include <atomic>
#include <flat_map>
#include <flat_set>
#include <iterator>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

namespace abmoid {

struct entity {
  using id_type = uint_fast32_t;
  id_type id;

  static entity create() {
    static std::atomic<id_type> global_id{1};
    return entity{global_id.fetch_add(1)};
  }
};

}

template <>
struct std::hash<abmoid::entity> {
  uint32_t operator()(abmoid::entity entity) const noexcept {
    return std::hash<decltype(entity.id)>{}(entity.id);
  }
};

namespace abmoid {

template <typename Component>
class entity_component {
  std::flat_map<entity, Component> storage;
};

struct mean {
  double value;
};

template <typename DistImpl>
class distribution : std::ranges::view_interface<distribution<DistImpl>> {
  std::mt19937& gen;
  DistImpl dist;

public:
  class iterator;

  template <typename ...Args>
  distribution(std::mt19937& gen, Args&& ...args)
    : gen(gen),
      dist(std::forward<Args>(args)...)
  { }

  iterator begin();
  auto end() {
    return std::unreachable_sentinel;
  }
};

template <typename DistImpl>
class distribution<DistImpl>::iterator {
  friend class distribution<DistImpl>;
  using Range = distribution<DistImpl>;

  Range* range;
  DistImpl::result_type value;

  DistImpl::result_type get_next() {
    return range->dist(range->gen);
  }

  iterator(Range* range)
    : range(range),
      value(get_next())
  { }

public:
  using difference_type = std::ptrdiff_t;
  using value_type = DistImpl::result_type;

  value_type operator*() const {
    return value; 
  }

  iterator& operator++() {
    value = get_next();
    return *this;
  }

  void operator++(int) {
    ++*this;
  }
};

using weibull_dist = distribution<std::weibull_distribution<>>;
using exponential_dist = distribution<std::exponential_distribution<>>;

static_assert(std::input_iterator<weibull_dist::iterator>);
static_assert(std::input_iterator<exponential_dist::iterator>);


template <typename DistImpl>
distribution<DistImpl>::iterator distribution<DistImpl>::begin() {
  return distribution<DistImpl>::iterator(this);
}

}  // namespace abmoid

struct infected_state {
  // Frames until recovered.
  unsigned timer;
};

struct agent_sir_model {
  double gamma;
  double beta;
  double beta_star;
  double recover_mean;
  unsigned contact_factor;
  std::mt19937 gen;
  std::vector<entity> population;
  std::flat_set<entity> susceptible;
  std::flat_map<entity, infected_state> infected;
  std::flat_set<entity> recovered;

  agent_sir_model(unsigned N, double gamma, double beta,
                  unsigned contact_factor)
    : gamma(gamma),
      beta(beta),
      beta_star(beta * 1.0 / contact_factor),
      recover_mean(gen, 1.0 / gamma),
      contact_factor(contact_factor),
      gen(),
      population(N)
  {
    // Initialize population.
    for (entity& e : population)
      e = entity::create();
    // TODO Initialize susceptibles and infecteds.
  }

  void update_susceptible() {
    // Iterate susceptibles and possibly make sick.
    for (auto itr = susceptible.begin(); itr != susceptible.end();) {
      size_t n = population.size();
      bool contact = false;
      // Choose 3 randos and check for infectedness.
      for (unsigned i = 0; i < contact_factor; ++i) {
        entity e = population[gen() % n];
        if (infected.contains(e))
          contact = contact || infected.contains(e);
      }
      // TODO Use beta_star to determine infection.
      if (contact) {
        // TODO Use random timer value (exp_dist(1 / gamma) * 1000)
        abmoid::exponential_dist dist(gen, recover_mean);
        infected[*itr] = infected_state{std::floor(dist() * 1000.0)};
        itr = susceptible.erase(itr);
      } else {
        ++itr;
      }
    }
  }

  void update_infected() {
    // Iterate infecteds updating timer and possibly make recovered.
    for (auto itr = infected.begin(); itr != infected.end();) {
      auto [ent, state] = *itr;
      if (state.timer == 0) {
        recovered.insert(ent);
        infected.erase(itr);
      } else {
        ++itr;
      }
    }
  }

  void update_recovered() {
    // Do nothing.
  }

  // Each frame we call update.
  void update() {
    update_susceptible();
    update_infected();
    update_recovered();
  }

};


int main() {
  std::mt19937 gen;
  int const N = 1000;
  std::vector<double> results(N, 0.0);
  auto mu_values = std::array<double, 4>{{0.5, 1.0, 1.5, 5.0}};

  // Plot stuff.

  for (auto [i, mu] : std::views::enumerate(mu_values)) {
    auto figure = matplot::figure(true);
    //matplot::title("Expontential Distribution");
    matplot::title(std::string("Exponential Distribution (mu = ") +
                   std::to_string(mu) +
                   ")");
    matplot::hold(matplot::on);

    // Simulate stuff.
    abmoid::exponential_dist dist(gen, 1.0 / mu);
    for (auto [result, value] : std::views::zip(results, dist))
      result = value;

    auto plot1 = matplot::hist(results, 50);
    plot1->normalization(matplot::histogram::normalization::pdf);
    plot1->line_width(2);

    matplot::save(std::string("img/exponential_dist_") +
                  std::to_string(i + 1) +
                  ".png");
    matplot::hold(matplot::off);
  }
}
