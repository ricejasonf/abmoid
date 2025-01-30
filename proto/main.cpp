#include <iterator>
#include <matplot/matplot.h>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

namespace abmoid {
class weibull_dist_iterator;

class weibull_dist : std::ranges::view_interface<weibull_dist> {
  friend class weibull_dist_iterator;
  using storage_type = std::vector<double>;

  std::mt19937 gen;
  std::weibull_distribution<> dist;
  // Store a run of the values in a vector.
  // (This is just an idea to be tested later.)
  storage_type values;

public:
  using difference_type = storage_type::difference_type;
  using value_type = storage_type::value_type;

  weibull_dist(double shape, double scale, size_t run_size)
    : gen(),
      dist(shape, scale),
      values(run_size)
  {
    std::cout << "shape: " << shape << '\n';
  }

  weibull_dist_iterator begin();
  auto end() {
    return std::unreachable_sentinel;
  }
};

class weibull_dist_iterator {
  friend class weibull_dist;
  using storage_type = weibull_dist::storage_type;

  weibull_dist* range;
  storage_type::iterator values_itr;

  storage_type::iterator repopulate() {
    // TODO Use C++26 std::ranges::generate_random.
    // std::ranges::generate_random(dist.values, range);
    for (double& value : range->values)
      value = range->dist(range->gen);
    return range->values.begin();
  }

  weibull_dist_iterator(weibull_dist* range)
    : range(range),
      values_itr(repopulate())
  { }

public:
  using difference_type = storage_type::difference_type;
  using value_type = storage_type::value_type;

  value_type operator*() const {
    return *values_itr; 
  }

  weibull_dist_iterator& operator++() {
    values_itr++;
    if (values_itr == range->values.end())
      values_itr = repopulate();
    return *this;
  }

  void operator++(int) {
    ++*this;
  }
};

static_assert(std::input_iterator<weibull_dist_iterator>);


weibull_dist_iterator weibull_dist::begin() {
  return weibull_dist_iterator(this);
}
}


int main() {
  int const N = 1000;
  std::vector<double> results(N, 0.0);
  auto k_values = std::array<double, 4>{{0.5, 1.0, 1.5, 5.0}};

  // Plot stuff

  for (auto [i, k] : std::views::enumerate(k_values)) {
    auto figure = matplot::figure(true);
    //matplot::title("Weibull Distribution");
    matplot::title(std::string("Weibull Distribution (k = ") +
                   std::to_string(k) +
                   ")");
    matplot::hold(matplot::on);
    abmoid::weibull_dist dist(k, 1.0, 50);
    for (auto [result, value] : std::views::zip(results, dist))
      result = value;

    auto plot1 = matplot::hist(results, 50);
    plot1->normalization(matplot::histogram::normalization::pdf);
    plot1->line_width(2);

    matplot::save(std::string("img/weibull_dist_") +
                  std::to_string(i + 1) +
                  ".png");
    matplot::hold(matplot::off);
  }
}
