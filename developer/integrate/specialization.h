#pragma once

#include <functional>
#include <map>
#include <tuple>

#include "control.h"

template<typename T, T M>
struct typeify {
  using TYPE = T;
  static constexpr T value = M;
};

template<size_t PHASE>
using phase_ = typeify<size_t, PHASE>;

enum simulation_phases_t : size_t {
  initialize,
  advance,
  analyze,
  io,
  mesh,
  finalize
}; // enum phases_t

enum target_attributes_t : size_t {
  time_advance_half = 0x01,
  time_advance_whole = 0x02,
  updated_eos_at_faces = 0x04
}; // enum target_attributes_t

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

  using cycle = cycle__<run_simulation,
    phase_<advance>,
    phase_<analyze>,
    phase_<io>,
    phase_<mesh>>;

  using phases = std::tuple<
    phase_<initialize>,
    cycle,
    phase_<finalize>
  >;

}; // runtime_policy_t

using control_t = flecsi::control::control__<runtime_policy_t>;
