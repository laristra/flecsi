#pragma once

#include <io-poc/control/control.h>
#include <unistd.h>

using namespace io_poc;

int init_species(int argc, char ** argv) {
  usleep(200000);
  std::cout << "initialize: init_species" << std::endl;
  std::cout << std::endl;
  return 0;
} // init_fields

register_action(initialize, init_species, init_species);
add_dependency(initialize, init_species, init_mesh);
