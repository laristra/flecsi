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

//----------------------------------------------------------------------------//
// Tasks
//----------------------------------------------------------------------------//

namespace example {

void particle_advance_task_a() {
  std::cout << "particle_advnace_task_a" << std::endl;
} // particle_advance_task_a

flecsi_register_task(particle_advance_task_a, example, loc, single);

void particle_advance_task_b() {
  std::cout << "particle_advnace_task_b" << std::endl;
} // particle_advance_task_b

flecsi_register_task(particle_advance_task_b, example, loc, single);

void particle_advance_task_c() {
  std::cout << "particle_advnace_task_c" << std::endl;
} // particle_advance_task_c

flecsi_register_task(particle_advance_task_c, example, loc, single);

} // namespace example

//----------------------------------------------------------------------------//
// Control Point
//----------------------------------------------------------------------------//

int advance_particles(int argc, char ** argv) {
  //usleep(200000);
  std::cout << "Executing advance_particles" << std::endl;
  flecsi_execute_task(particle_advance_task_a, example, single);
  flecsi_execute_task(particle_advance_task_b, example, single);
  flecsi_execute_task(particle_advance_task_c, example, single);
  return 0;
} // advance_particles

register_action(advance, advance_particles, advance_particles);
