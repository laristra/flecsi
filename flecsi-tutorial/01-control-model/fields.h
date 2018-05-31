/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include <flecsi-tutorial/specialization/control/control.h>
#include <unistd.h>

using namespace flecsi::tutorial;

int init_fields(int argc, char ** argv) {
  //usleep(200000);
  std::cout << "Executing init_fields" << std::endl;
  return 0;
} // init_fields

register_action(initialize, init_fields, init_fields);
add_dependency(initialize, init_fields, init_mesh);

int update_fields(int argc, char ** argv) {
  //usleep(200000);
  std::cout << "Executing advance particles" << std::endl;
  return 0;
} // update_fields

register_action(advance, update_fields, update_fields);
add_dependency(advance, update_fields, accumulate_currents);
