#pragma once

#include <io-poc/control/control.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace io_poc;

int
poynting_flux(int argc, char ** argv) {
  std::this_thread::sleep_for(std::chrono::microseconds(200));
  std::cout << "analyze: poynting_flux" << std::endl;
  return 0;
} // poynting_flux

flecsi_register_action(analyze /* phase */,
  poynting_flux /* name */,
  poynting_flux /* action */);
