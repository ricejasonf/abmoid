#ifndef ABMOID_RK4_HPP
#define ABMOID_RK4_HPP

#include <concepts>
#include <utility>

namespace abmoid {
using time_t = double;

template <typename F, std::SemiRegular State>
concept SystemFn = std::invocable<F, State, State&, time_t>;

template <typename F, std::SemiRegular State>
concept ResultVisitorFn = std::invocable<F, State, time_t>;

struct time_step {
  double value;
};

template <typename SystemFn,
          std::SemiRegular State,
          ResultVisitorFn ResultVisitorFn>
void rk4(SystemFn&& fn, ResultVisitorFn&& result, State initial_state,
         time_step dt_, std::size_t step_count) {
  if (step_count < 1)
    return {};

  using Value = typename integrator_output<dim>::value_type;
  auto output = integrator_output<dim>(step_count);
  auto dt = dt_.value;
  State prev_val = init.value;
  auto t = 0;
  for (int i = 0; i < step_count; ++i) {
    State const& x = prev_val;
    State k_1 = fn(t, x);
    State k_2 = fn(t, x + k_1 * (dt / 2.0));
    State k_3 = fn(t, x + k_2 * (dt / 2.0));
    State k_4 = fn(t, x + k_3 * dt);
    
    prev_val = prev_val + dt / 6.0 * (k_1 + 2.0 * k_2 + 2.0 * k_3 + k_4);
    result(x, t);
    t += dt;
  }
}

}  // namespace abmoid

#endif
