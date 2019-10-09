#pragma once

#include <io-poc/control/control.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace io_poc;

int
advance_particles(int argc, char ** argv) {
  std::this_thread::sleep_for(std::chrono::microseconds(200));
  std::cout << "advance: advance_particles" << std::endl;
  return 0;
} // advance_particles

flecsi_register_action(advance, advance_particles, advance_particles);
