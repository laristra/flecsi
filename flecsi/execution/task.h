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

/*! @file */

#include <iostream>
#include <string>

#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/utils/static_verify.h>

namespace flecsi {
namespace execution {

/*!
  The task_interface__ type provides a high-level task interface that is
  implemented by the given execution policy.

  @tparam EXECUTION_POLICY The backend execution policy.

  @ingroup execution
 */

template<typename EXECUTION_POLICY>
struct task_interface__ {

  /*!
    The runtime_state_t type stores runtime-specific state information
    that is required to execute a user task. This is only needed for
    the functor interface, which is currently not in use.
   */

  using runtime_state_t = typename EXECUTION_POLICY::runtime_state_t;

  static runtime_state_t & runtime_state(void * task) {
    return EXECUTION_POLICY::runtime_state(task);
  } // runtime_state

  /*!
    Register a task with the FleCSI runtime.

    @tparam KEY       A hash key identifying the task.
    @tparam RETURN    The return type of the user task.
    @tparam ARG_TUPLE A std::tuple of the user task argument types.
    @tparam DELEGATE  The delegate function that invokes the user task.

    @param processor The processor type.
    @param launch    The launch flags.
    @param name      The string identifier of the task.

    @return The return type for task registration is determined by
            the specific backend runtime being used.
   */

  template<
      size_t KEY,
      typename RETURN,
      typename ARG_TUPLE,
      RETURN (*DELEGATE)(ARG_TUPLE)>
  static decltype(auto)
  register_task(processor_type_t processor, launch_t launch, std::string name) {
    return EXECUTION_POLICY::template register_task<
        KEY, RETURN, ARG_TUPLE, DELEGATE>(processor, launch, name);
  } // register_task

  /*!
    Execute a task.

    @tparam RETURN The return type of the task.
    @tparam ARG_TUPLE A std::tuple of the user task argument types.
    @tparam ARGS The task arguments.

    @param launch The launch mode for this task execution.
    @param args   The arguments to pass to the user task during execution.
   */

  template<
      launch_type_t launch,
      size_t KEY,
      typename RETURN,
      typename ARG_TUPLE,
      typename... ARGS>
  static decltype(auto) execute_task(ARGS &&... args) {
    return EXECUTION_POLICY::template execute_task<
        launch, KEY, RETURN, ARG_TUPLE>(std::forward<ARGS>(args)...);
  } // execute_task

  /*!
    Register a custom reduction operation.

    @tparam NAME      A hash key identifying the operation.
    @tparam OPERATION The user-defined operation type. The interface
                      for this type is prescribed and is statically
                      checked at this level.
   */

  template<
    size_t NAME,
    typename OPERATION>
  static decltype(auto)
  register_reduction_operation() {

    // FIXME: Add static check of OPERATION type

    return EXECUTION_POLICY::template register_reduction_operation<
      NAME, OPERATION>();
  } // register_reduction_operation

}; // struct task_interface__

template<typename TYPE>
struct reduce_sum {
  TYPE apply(TYPE & lhs, TYPE rhs) {
    return lhs += rhs;
  } // apply
}; // struct reduce_sum

template<typename TYPE>
struct reduce_min {
  TYPE apply(TYPE & lhs, TYPE rhs) {
    return lhs < rhs ? lhs : rhs;
  } // apply
}; // struct reduce_min

template<typename TYPE>
struct reduce_max {
  TYPE apply(TYPE & lhs, TYPE rhs) {
    return lhs > rhs ? lhs : rhs;
  } // apply
}; // struct reduce_max

} // namespace execution
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_EXECUTION_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/flecsi_runtime_execution_policy.h>

namespace flecsi {
namespace execution {

/*!
  The task_interface_t type is the high-level interface to the FleCSI
  task model.

  @ingroup execution
 */

using task_interface_t = task_interface__<FLECSI_RUNTIME_EXECUTION_POLICY>;

/*!
  Use the execution policy to define the future type.

  @tparam RETURN The return type of the associated task.

  @ingroup execution
 */

template<typename RETURN, launch_type_t launch>
using future__ = FLECSI_RUNTIME_EXECUTION_POLICY::future__<RETURN, launch>;

//----------------------------------------------------------------------------//
// Static verification of public future interface for type defined by
// execution policy.
//----------------------------------------------------------------------------//

namespace verify_future {

#if 0
FLECSI_MEMBER_CHECKER(wait);
FLECSI_MEMBER_CHECKER(get);

static_assert(
    verify_future::has_member_wait<future__<double>>::value,
    "future type missing wait method");

static_assert(
    verify_future::has_member_get<future__<double>>::value,
    "future type missing get method");
#endif

} // namespace verify_future

} // namespace execution
} // namespace flecsi
