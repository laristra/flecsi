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

//----------------------------------------------------------------------------//
// Tasks
//----------------------------------------------------------------------------//

namespace example {

void mesh_init_task() {
  std::cout << "init_task" << std::endl;
} // mesh_init_task

flecsi_register_task(mesh_init_task, example, loc, single);

void mesh_fixup_task() {
  std::cout << "fixup_task" << std::endl;
} // mesh_init_task

flecsi_register_task(mesh_fixup_task, example, loc, single);

} // namespace example

//----------------------------------------------------------------------------//
// Control Points
//----------------------------------------------------------------------------//

int init_mesh(int argc, char ** argv) {
  //usleep(200000);
  std::cout << "Executing init_mesh" << std::endl;
  flecsi_execute_task(mesh_init_task, example, single);
  return 0;
} // init_mesh

register_action(initialize, init_mesh, init_mesh);

int fixup_mesh(int argc, char ** argv) {
  //usleep(200000);
  std::cout << "Executing fixup_mesh" << std::endl;
  flecsi_execute_task(mesh_fixup_task, example, single);
  return 0;
} // fixup_mesh

register_action(mesh, fixup_mesh, fixup_mesh);
