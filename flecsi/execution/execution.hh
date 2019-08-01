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

#include <flecsi/execution/context.hh>
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

/*----------------------------------------------------------------------------*
  Global object interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_add_global_object

  Add a global object to the context. Global objects cannot be added
  from within a task. Attempts to do so will generate a runtime error.

  @param index  The size_t index of the global object within the given scope.
  @param scope  The string scope of the global object.
  @param type   The C++ type of the global object.
  @param ...    A variadic argument list of the runtime arguments to the
                constructor.

  @ingroup execution
 */

#define flecsi_add_global_object(index, scope, type, ...)                      \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::context_t::instance()                                     \
    .template add_global_object<flecsi_internal_string_hash(scope), type>(     \
      index, ##__VA_ARGS__);

/*!
  @def flecsi_get_global_object

  Get a global object instance.

  @param index  The size_t index of the global object within the given scope.
  @param scope  The string scope of the global object.
  @param type   The type of the global object.

  @ingroup execution
 */

#define flecsi_get_global_object(index, scope, type)                           \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::context_t::instance()                                     \
    .template get_global_object<flecsi_internal_string_hash(scope), type>(     \
      index);
