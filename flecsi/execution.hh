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

#include <flecsi/execution/backend.hh>
#include <flecsi/execution/internal.hh>
#include <flecsi/execution/launch.hh>
#include <flecsi/execution/task_attributes.hh>
#include <flecsi/runtime/backend.hh>
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
  return runtime::context_t::instance().process();
}

/*!
  Return the number of processes.
 */

inline size_t
processes() {
  return runtime::context_t::instance().processes();
}

/*!
  Return the number of threads per process.
 */

inline size_t
threads_per_process() {
  return runtime::context_t::instance().threads_per_process();
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
  return runtime::context_t::instance().threads();
}

/*!
  Return the color of the current execution instance. This function is only
  valid if invoked from within a task.
 */

inline size_t
color() {
  return runtime::context_t::instance().color();
}

/*!
  Return the number of colors of the current task invocation. This function is
  only valid if invoked from within a task.
 */

inline size_t
colors() {
  return runtime::context_t::instance().colors();
}

/*!
  Execute a reduction task.

  @tparam TASK                The user task.
  @tparam LAUNCH_DOMAIN       The launch domain.
  @tparam REDUCTION_OPERATION The reduction operation.
  @tparam ATTRIBUTES          The task attributes mask.
  @tparam ARGS                The user-specified task arguments.
 */

template<auto & TASK,
  size_t LAUNCH_DOMAIN,
  size_t REDUCTION_OPERATION,
  size_t ATTRIBUTES,
  typename... ARGS>
decltype(auto) reduce(ARGS &&... args);

/*!
  Execute a task.

  @tparam TASK          The user task.
    Its parameters may be of any default-constructible,
    trivially-move-assignable, non-pointer type, any type that supports the
    Legion return-value serialization interface, or any of several standard
    containers of such types.
  @tparam LAUNCH_DOMAIN The launch domain id.
  @tparam ATTRIBUTES    The task attributes mask.
  @tparam ARGS The user-specified task arguments, implicitly converted to the
    parameter types for \a TASK.

  \note Additional types may be supported by defining appropriate
    specializations of \c utils::serial or \c utils::serial_convert.  Avoid
    passing large objects to tasks repeatedly; use global variables (and,
    perhaps, pass keys to select from them) or fields.
 */

template<auto & TASK,
  size_t LAUNCH_DOMAIN = flecsi::index,
  size_t ATTRIBUTES = flecsi::loc | flecsi::leaf,
  typename... ARGS>
decltype(auto)
execute(ARGS &&... args) {
  return reduce<TASK, LAUNCH_DOMAIN, flecsi_internal_hash(0), ATTRIBUTES>(
    std::forward<ARGS>(args)...);
} // execute

} // namespace flecsi
