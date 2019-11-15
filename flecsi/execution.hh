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
#include <flecsi/execution/launch.hh>
#include <flecsi/execution/reduction.hh>
#include <flecsi/execution/task_attributes.hh>
#include <flecsi/runtime/backend.hh>

/*----------------------------------------------------------------------------*
  Basic runtime interface
 *----------------------------------------------------------------------------*/

namespace flecsi {

/*!
  Perform FleCSI runtime initialization. If \em dependent is true, this call
  will also initialize any runtime on which FleCSI depends.

  @param argc      The number of command-line arguments.
  @param argv      The command-line arguments.
  @param dependent A boolean telling FleCSI whether or not to initialize
                   runtimes on which it depends.

  @return An integer indicating the initialization status. This may be
          interpreted as a \em flecsi::runtime::status enumeration, e.g.,
          a value of 1 is equivalent to flecsi::runtime::status::help.
 */

inline int
initialize(int argc, char ** argv, bool dependent = true) {
  return runtime::context_t::instance().initialize(argc, argv, dependent);
}

/*!
  Perform FleCSI runtime start. This causes the runtime to begin execution
  of the top-level action.

  @return An integer indicating the finalization status. This will either
          be 0 for successful completion, or an error code from
          flecsi::runtime::status.
 */

inline int
start() {
  return runtime::context_t::instance().start();
}

/*!
  Perform FleCSI runtime finalization. If FleCSI was initialized with the \em
  dependent flag set to true, FleCSI will also finalize any runtimes on which
  it depends.

  @return An integer indicating the finalization status. This will either
          be 0 for successful completion, or an error code from
          flecsi::runtime::status.
 */

inline int
finalize() {
  return runtime::context_t::instance().finalize();
}

/*!
  Add a program option to the \em options_description identified by \em
  label. If the options_description does not exist, it is created.

  @param label A std::string containing the options_description label.

  @code
  auto my_unique_variable_name = add_program_option("Description Label",
    arg0, arg1, ...);
  @endcode

  @return A boolean value that can be used at namespace scope for assignment.
 */

template<typename... ARGS>
inline bool
add_program_option(std::string const & label, ARGS &&... args) {
  return runtime::context_t::instance().add_program_option(
    label, std::forward<ARGS>(args)...);
}

/*!
  Return the boost program options variable map.

  @note The \em initialize method must be called before this method can be
  invokded.
 */

inline boost::program_options::variables_map const &
program_options_variables_map() {
  return runtime::context_t::instance().program_options_variables_map();
}

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
  @tparam REDUCTION_OPERATION The reduction operation type.
  @tparam ATTRIBUTES          The task attributes mask.
  @tparam ARGS                The user-specified task arguments.
 */

template<auto & TASK,
  const execution::launch_domain & LAUNCH_DOMAIN,
  class REDUCTION_OPERATION,
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
    If \a ATTRIBUTES specifies an MPI task, parameters need merely be movable.
  @tparam LAUNCH_DOMAIN The launch domain object.
  @tparam ATTRIBUTES    The task attributes mask.
  @tparam ARGS The user-specified task arguments, implicitly converted to the
    parameter types for \a TASK.

  \note Additional types may be supported by defining appropriate
    specializations of \c utils::serial or \c utils::serial_convert.  Avoid
    passing large objects to tasks repeatedly; use global variables (and,
    perhaps, pass keys to select from them) or fields.
 */

template<auto & TASK,
  const execution::launch_domain & LAUNCH_DOMAIN = flecsi::index,
  size_t ATTRIBUTES = flecsi::loc | flecsi::leaf,
  typename... ARGS>
decltype(auto)
execute(ARGS &&... args) {
  return reduce<TASK, LAUNCH_DOMAIN, void, ATTRIBUTES>(
    std::forward<ARGS>(args)...);
} // execute

} // namespace flecsi
