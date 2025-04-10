#include <array>
#include <iostream>
#include <thread>

#include "peak_times.hpp"

int main() {

  auto run = [](unsigned seed) {
    peak_times::result_set results;
    unsigned A_N = 5'000;
    unsigned B_N = 5'000;
    unsigned Br_max = 1005;
    unsigned Br_step = 100;

    for (unsigned Br = 5; Br < Br_max; Br += Br_step) {
      unsigned B_N = 10'000 - A_N;
      peak_times::peak_result peaks = peak_times::run_model(Br, A_N, B_N, seed);
      results.push_back({
        .Br = Br,
        .A_N = A_N,
        .B_N = B_N,
        .A_I_max_t = peaks[0].t,
        .B_I_max_t = peaks[1].t
      });
    }
    return results;
  };

  unsigned iterations = 0;
  auto handle = [&](peak_times::result_set const& results) {
    peak_times::print_result_set(results);
    std::cout << "\n\n";  // Begin new dataset.
    ++iterations;
    std::cerr << "Finished simulation #" << iterations << '\n';
  };

  unsigned max_threads = std::thread::hardware_concurrency();

  peak_times::run_experiments(100, max_threads, run, handle);
}
