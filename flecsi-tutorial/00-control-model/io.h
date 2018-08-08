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

#include <flecsi-config.h>

#include <flecsi/utils/graphviz.h>
#include <flecsi-tutorial/specialization/control/control.h>
#include <unistd.h>

using namespace flecsi::tutorial;

int restart_dump(int argc, char ** argv) {
  usleep(200000);
  std::cout << "io: restart_dump" << std::endl;
  return 0;
} // restart_dump

register_action(io /* phase */,
  restart_dump /* name */,
  restart_dump /* action */);

int output_final(int argc, char ** argv) {

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  usleep(200000);
  std::cout << "finalize: output_final" << std::endl;

  if(flecsi_color() == 0) {
    auto & control = control_t::instance();
    flecsi::utils::graphviz_t gv;
    control.write(gv);
    gv.write("control.gv");
  } // if
#endif

  return 0;
} // finalize

register_action(finalize, output_final, output_final);
