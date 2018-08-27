#pragma once

#include <io-poc/control/control.h>
#include <unistd.h>

using namespace io_poc;

int poynting_flux(int argc, char ** argv) {
  usleep(200000);
  std::cout << "analyze: poynting_flux" << std::endl;
  return 0;
} // poynting_flux

register_action(analyze /* phase */,
  poynting_flux /* name */,
  poynting_flux /* action */);
