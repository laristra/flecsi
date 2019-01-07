#pragma once

#include <io-poc/control/control.h>
#include <unistd.h>

using namespace io_poc;

int init_fields(int argc, char ** argv) {
  usleep(200000);
  std::cout << "initialize: init_fields" << std::endl;
  return 0;
} // init_fields

register_action(
  initialize /* phase */,
  init_fields /* action name */,
  init_fields /* action */
);

add_dependency(
  initialize /* phase */,
  init_fields /* to */,
  init_mesh /* from */
);

int update_fields(int argc, char ** argv) {
  usleep(200000);
  std::cout << "advance: update_fields" << std::endl;

  if(check_attribute(
    advance /* phase */,
    accumulate_currents /* action */,
    time_advance_half /* attribute */)
  ) {
    std::cout << "\tadvancing half" << std::endl;
  }
  else {
    std::cout << "\tadvancing whole" << std::endl;
  } // if

  return 0;
} // update_fields

register_action(advance, update_fields, update_fields);
add_dependency(advance, update_fields, accumulate_currents);
