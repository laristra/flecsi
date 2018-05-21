#pragma once

#include <functional>
#include <map>
#include <tuple>

#include "control.h"

enum simulation_phases_t : size_t {
  initialize,
  advance,
  analyze,
  io,
  mesh,
  finalize
}; // enum phases_t

struct runtime_policy_t {

  static bool run_simulation() {
    return context_t::instance().run(5);
  } // run_simulation

  template<bool (*CRITERIUM)(), typename ... PHASES>
  struct cycle__ {

    using TYPE = std::tuple<PHASES ...>;

    static bool run() {
      return CRITERIUM();
    } // run

  }; // struct cycle__

  using cycle_t = cycle__<run_simulation,
    phase_<advance>,
    phase_<analyze>,
    phase_<io>,
    phase_<mesh>>;

  using phases_t = std::tuple<
    phase_<initialize>,
    cycle_t,
    phase_<finalize>
  >;

}; // runtime_policy_t

using control_t = control__<runtime_policy_t>;
