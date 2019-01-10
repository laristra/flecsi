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

namespace flecsi {
namespace execution {

/*!
  The task_interface_u type provides a high-level task interface that is
  implemented by the given execution policy.

  @tparam EXECUTION_POLICY The backend execution policy.

  @ingroup execution
 */

template<typename EXECUTION_POLICY>
struct task_interface_u {

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

  template<size_t KEY,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
  static decltype(auto)
  register_task(processor_type_t processor, launch_t launch, std::string name) {
    return EXECUTION_POLICY::template register_task<KEY, RETURN, ARG_TUPLE,
      DELEGATE>(processor, launch, name);
  } // register_task

  /*!
    Execute a task.

    @tparam LAUNCH    The launch mode for this task execution.
    @tparam TASK      The task hash key.
    @tparam REDUCTION The reduction operation hash key.
    @tparam RETURN    The return type of the task.
    @tparam ARG_TUPLE A std::tuple of the user task argument types.
    @tparam ARGS      The task arguments.

    @param args   The arguments to pass to the user task during execution.
   */

  template<launch_type_t LAUNCH,
    size_t TASK,
    size_t REDUCTION,
    typename RETURN,
    typename ARG_TUPLE,
    typename... ARGS>
  static decltype(auto) execute_task(ARGS &&... args) {
    return EXECUTION_POLICY::template execute_task<LAUNCH, TASK, REDUCTION,
      RETURN, ARG_TUPLE>(std::forward<ARGS>(args)...);
  } // execute_task

  /*!
    Register a custom reduction operation.

    @tparam HASH A hash key identifying the operation.
    @tparam TYPE The user-defined operation type. The interface
                 for this type is prescribed.
   */

  template<size_t HASH, typename TYPE>
  static decltype(auto) register_reduction_operation() {
    return EXECUTION_POLICY::template register_reduction_operation<
      HASH, TYPE>();
  } // register_reduction_operation

}; // struct task_interface_u

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

using task_interface_t = task_interface_u<FLECSI_RUNTIME_EXECUTION_POLICY>;

/*!
  Use the execution policy to define the future type.

  @tparam RETURN The return type of the associated task.

  @ingroup execution
 */

//template<typename RETURN, launch_type_t launch>
//using future_u = FLECSI_RUNTIME_EXECUTION_POLICY::future_u<RETURN, launch>;

} // namespace execution
} // namespace flecsi
