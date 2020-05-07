/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include "specialization/control.hh"

namespace standalone {

int initialize() {
  flog(info) << __func__ << std::endl;
  return 0;
}

control::action<initialize, cp::initialize> initialize_action;

int advance() {
  flog(info) << __func__ << std::endl;
  return 0;
}

control::action<advance, cp::advance> advance_action;

int analyze() {
  flog(info) << __func__ << std::endl;
  return 0;
}

control::action<analyze, cp::analyze> analyze_action;

int finalize() {
  flog(info) << __func__ << std::endl;
  return 0;
}

control::action<finalize, cp::finalize> finalize_action;

} // namespace standalone
