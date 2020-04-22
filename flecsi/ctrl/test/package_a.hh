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

#include "flecsi/execution.hh"
#include "flecsi/flog.hh"

namespace package_a {

//----------------------------------------------------------------------------//
// Initialization
//----------------------------------------------------------------------------//

inline int
init() {
  flog(info) << "A init" << std::endl;
  return 0;
}

inline control::action<init, cp::initialization> init_action;

//----------------------------------------------------------------------------//
// Advance
//----------------------------------------------------------------------------//

inline void
task1() {
  flog(trace) << "A task 1" << std::endl;
}

inline void
task2() {
  flog(trace) << "A task 2" << std::endl;
}

inline int
advance() {
  flog(info) << "A advance" << std::endl;
  flecsi::execute<task1>();
  flecsi::execute<task2>();
  return 0;
}

inline control::action<advance, cp::advance> advance_action;

//----------------------------------------------------------------------------//
// Subcycle
//----------------------------------------------------------------------------//

inline void
subcycle_task() {
  flog(trace) << "A subcycle task" << std::endl;
}

inline int
subcycle() {
  flecsi::execute<subcycle_task>();
  return 0;
}

inline control::action<subcycle, cp::advance_subcycle> subcycle_action;

//----------------------------------------------------------------------------//
// Analysis
//----------------------------------------------------------------------------//

inline void
task3() {
  flog(trace) << "A task 3" << std::endl;
}

inline void
task4() {
  flog(trace) << "A task 4" << std::endl;
}

inline int
analyze() {
  flog(info) << "A analyze" << std::endl;
  flecsi::execute<task3>();
  flecsi::execute<task4>();
  return 0;
}

inline control::action<analyze, cp::analyze> analyze_action;

//----------------------------------------------------------------------------//
// I/O
//----------------------------------------------------------------------------//

inline int
io() {
  flog(info) << "A I/0" << std::endl;
  return 0;
}

inline control::action<io, cp::io> io_action;

//----------------------------------------------------------------------------//
// Mesh
//----------------------------------------------------------------------------//

inline int
mesh() {
  flog(info) << "A mesh" << std::endl;
  return 0;
}

inline control::action<mesh, cp::mesh> mesh_action;

//----------------------------------------------------------------------------//
// Finalize
//----------------------------------------------------------------------------//

inline int
finalize() {
  flog(info) << "A finalize" << std::endl;
  return 0;
}

inline control::action<finalize, cp::finalization> finalize_action;

} // namespace package_a
