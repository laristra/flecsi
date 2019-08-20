#pragma once

#include <io-poc/control/control.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace io_poc;

int
init_species(int argc, char ** argv) {
  std::this_thread::sleep_for(std::chrono::microseconds(200));
  std::cout << "initialize: init_species" << std::endl;
  std::cout << std::endl;
  return 0;
} // init_fields

register_action(initialize, init_species, init_species);
add_dependency(initialize, init_species, init_mesh);
