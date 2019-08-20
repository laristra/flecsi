#pragma once

#include <io-poc/control/control.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace io_poc;

int
init_mesh(int argc, char ** argv) {
  std::this_thread::sleep_for(std::chrono::microseconds(200));
  std::cout << "initialize: init_mesh" << std::endl;
  return 0;
} // init_mesh

register_action(initialize, init_mesh, init_mesh);

int
fixup_mesh(int argc, char ** argv) {
  std::this_thread::sleep_for(std::chrono::microseconds(200));
  std::cout << "mesh: fixup_mesh" << std::endl;
  std::cout << std::endl;
  return 0;
} // fixup_mesh

register_action(mesh, fixup_mesh, fixup_mesh);
