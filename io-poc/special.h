#pragma once

#include <io-poc/control/control.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace io_poc;

int
particle_special(int argc, char ** argv) {
  std::this_thread::sleep_for(std::chrono::microseconds(200));
  std::cout << "advance: particle_special" << std::endl;
  return 0;
} // accumulate_currents

#if defined(ENABLE_SPECIAL)
flecsi_register_action(advance, particle_special, particle_special);
add_dependency(advance, particle_special, advance_particles);
add_dependency(advance, accumulate_currents, particle_special);
#endif
