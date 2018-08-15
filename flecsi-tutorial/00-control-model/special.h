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

int particle_special(int argc, char ** argv) {
  usleep(200000);
  std::cout << "advance: particle_special" << std::endl;
  return 0;
} // accumulate_currents

#if defined(ENABLE_SPECIAL)
register_action(advance, particle_special, particle_special);
add_dependency(advance, particle_special, advance_particles);
add_dependency(advance, accumulate_currents, particle_special);
#endif
