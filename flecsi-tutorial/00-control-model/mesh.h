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

#include <flecsi/execution/execution.h>
#include <flecsi-tutorial/specialization/control/control.h>
#include <unistd.h>

using namespace flecsi::tutorial;

int init_mesh(int argc, char ** argv) {
  usleep(200000);
  std::cout << "initialize: init_mesh" << std::endl;
  return 0;
} // init_mesh

register_action(initialize, init_mesh, init_mesh);

int fixup_mesh(int argc, char ** argv) {
  usleep(200000);
  std::cout << "mesh: fixup_mesh" << std::endl;
  std::cout << std::endl;
  return 0;
} // fixup_mesh

register_action(mesh, fixup_mesh, fixup_mesh);
