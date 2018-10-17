#pragma once

#include <io-poc/control/control.h>
#include <unistd.h>

using namespace io_poc;

int accumulate_currents(int argc, char ** argv) {
  usleep(200000);
  std::cout << "advance: accumulate_currents" << std::endl;
  return 0;
} // accumulate_currents

register_action(advance, accumulate_currents, accumulate_currents,
  time_advance_half);
add_dependency(advance, accumulate_currents, advance_particles);
