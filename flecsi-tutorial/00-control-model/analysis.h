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

register_action(analyze /* phase */,
  poynting_flux /* name */,
  poynting_flux /* action */);
