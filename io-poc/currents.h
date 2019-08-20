#pragma once

#include <io-poc/control/control.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace io_poc;

int
accumulate_currents(int argc, char ** argv) {
  std::this_thread::sleep_for(std::chrono::microseconds(200));
  std::cout << "advance: accumulate_currents" << std::endl;
  return 0;
} // accumulate_currents

register_action(advance,
  accumulate_currents,
  accumulate_currents,
  time_advance_half);
add_dependency(advance, accumulate_currents, advance_particles);
