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

int poynting_flux(int argc, char ** argv) {
  usleep(200000);
  std::cout << "analyze: poynting_flux" << std::endl;
  return 0;
} // poynting_flux

register_action(analyze, poynting_flux, poynting_flux);

int output_final(int argc, char ** argv) {
  usleep(200000);
  std::cout << "finalize: output_final" << std::endl;

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  auto & control = control_t::instance();
  flecsi::utils::graphviz_t gv;
  control.write(gv);
  gv.write("control.gv");
#endif

  return 0;
} // finalize

register_action(finalize, output_final, output_final);
