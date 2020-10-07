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

int
initialize() {
  flog(info) << __func__ << std::endl;
  control::state().steps() = 1000;
  return 0;
}

control::action<initialize, cp::initialize> initialize_action;

void
advance_task() {
  usleep(50000);
}

int
advance() {
  if(control::state().step() % 10 == 0) {
    flog(info) << __func__ << " step: " << control::state().step() << std::endl;
  } // if
  flecsi::execute<advance_task>();
  return 0;
}

control::action<advance, cp::advance> advance_action;

int
analyze() {
  return 0;
}

control::action<analyze, cp::analyze> analyze_action;

int
finalize() {
  flog(info) << __func__ << std::endl;
  return 0;
}

control::action<finalize, cp::finalize> finalize_action;

} // namespace standalone
