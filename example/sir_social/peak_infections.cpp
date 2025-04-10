#include <array>
#include <iostream>

#include "peak_times.hpp"

int main() {
  // unsigned max_threads = std::thread::hardware_concurrency();
  // Br - Number of bridge connections
  constexpr unsigned Br_max = 1'000;
  constexpr unsigned Br_step = 100;

  // A - The group "A" where the infection starts.
  constexpr unsigned A_N_max = 9'500;
  constexpr unsigned A_N_step = 500;

  unsigned iterations = 0;
  for (unsigned A_N = 9'500; A_N > 0; A_N -= A_N_step) {
    for (unsigned Br = 5; Br <= Br_max; Br += Br_step) {
      unsigned B_N = 10'000 - A_N;
      std::cerr << "Plotting" <<
                   " Br = " << Br << " A.N = " << A_N <<
                   " B.N = " << B_N <<
                   '\n';
      peak_times::peak_result peaks = peak_times::run_model(Br, A_N, B_N, 0);
      peak_times::print_result_row({
        .Br = Br,
        .A_N = A_N,
        .B_N = B_N,
        .A_I_max_t = peaks[0].t,
        .B_I_max_t = peaks[1].t
      });
    }
    std::cout << "\n\n";  // Begin new dataset.
    ++iterations;
    std::cerr << "Finished Iteration #" << iterations << '\n';
  }
}
