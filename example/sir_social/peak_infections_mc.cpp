#include <array>
#include <iostream>
#include <thread>

#include "peak_times.hpp"

int main() {
  // A - The group "A" where the infection starts.
  constexpr unsigned A_N_max = 9'500;
  constexpr unsigned A_N_step = 500;
  constexpr unsigned Br_max = 1005;
  constexpr unsigned Br_step = 25;

  unsigned iterations = 0;
  for (unsigned A_N = 500; A_N <= A_N_max; A_N += A_N_step) {
    auto run = [A_N](unsigned seed) {
      unsigned B_N = 10'000 - A_N;
      peak_times::result_set results;

      for (unsigned Br = 5; Br <= Br_max; Br += Br_step) {
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

    auto handle = [&](peak_times::result_set const& results) {
      peak_times::print_result_set(results);
      std::cout << "\n\n";  // Begin new dataset.
      std::cerr << "Finished simulation #" << iterations << '\n';
      ++iterations;
    };

    unsigned max_threads = std::thread::hardware_concurrency();

    peak_times::run_experiments(100, max_threads, run, handle);
  }
}
