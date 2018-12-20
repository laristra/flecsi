#include <flecsi/control/unit_control_policy.h>

#include <iostream>

using namespace flecsi::control;

int unit_init(int argc, char ** argv) {
  std::cout << "SHIT" << std::endl;
  return 0;
}

unit_register_action(initialize, unit_init, unit_init);
