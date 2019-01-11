#pragma once

#include <io-poc/control/control.h>
#include <unistd.h>

using namespace io_poc;

int advance_particles(int argc, char ** argv) {
  usleep(200000);
  std::cout << "advance: advance_particles" << std::endl;
  return 0;
} // advance_particles

register_action(advance, advance_particles, advance_particles);
