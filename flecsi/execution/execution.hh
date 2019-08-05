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

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#define __FLECSI_PRIVATE__
#endif

#include "flecsi/runtime/context_policy.hh"
#include <flecsi/execution/internal.hh>
#include <flecsi/execution/task.hh>
//#include <flecsi/execution/reduction.hh>

/*----------------------------------------------------------------------------*
  Basic runtime interface
 *----------------------------------------------------------------------------*/

namespace flecsi {

/*!
  Return the current process id.
 */

inline size_t
process() {
  return execution::context_t::instance().process();
}

/*!
  Return the number of processes.
 */

inline size_t
processes() {
  return execution::context_t::instance().processes();
}

/*!
  Return the number of threads per process.
 */

inline size_t
threads_per_process() {
  return execution::context_t::instance().threads_per_process();
}

/*!
  Return the number of execution instances with which the runtime was
  invoked. In this context a \em thread is defined as an instance of
  execution, and does not imply any other properties. This interface can be
  used to determine the full subscription of the execution instances of the
  running process that invokded the FleCSI runtime.
 */

inline size_t
threads() {
  return execution::context_t::instance().threads();
}

/*!
  Return the color of the current execution instance. This function is only
  valid if invoked from within a task.
 */

inline size_t
color() {
  return execution::context_t::instance().color();
}

/*!
  Return the number of colors of the current task invocation. This function is
  only valid if invoked from within a task.
 */

inline size_t
colors() {
  return execution::context_t::instance().colors();
}

} // namespace flecsi
