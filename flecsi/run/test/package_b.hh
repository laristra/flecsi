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

#include "cycle_control.hh"
#include "package_a.hh"

#include "flecsi/flog.hh"

namespace package_b {

//----------------------------------------------------------------------------//
// Advance
//----------------------------------------------------------------------------//

inline int
advance() {
  flog(info) << "B advance" << std::endl;
  return 0;
} // advance

inline control::action<advance, cp::advance> advance_action;
inline const auto dep_a = advance_action.add(package_a::advance_action);

//----------------------------------------------------------------------------//
// Subcycle
//----------------------------------------------------------------------------//

inline void
subcycle_task() {
  flog(trace) << "B subcycle task" << std::endl;
}

inline int
subcycle() {
  flecsi::execute<subcycle_task>();
  return 0;
}

inline control::action<subcycle, cp::advance_subcycle> subcycle_action;
inline const auto dep_a_sub = subcycle_action.add(package_a::subcycle_action);

//----------------------------------------------------------------------------//
// Analyze
//----------------------------------------------------------------------------//

inline int
analyze() {
  flog(info) << "B analyze" << std::endl;
  return 0;
} // analyze

inline control::action<analyze, cp::analyze> analyze_action;

} // namespace package_b
