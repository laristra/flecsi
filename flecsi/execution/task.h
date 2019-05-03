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
#error Do not include this file directly!
#else
#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#endif

#include <iostream>
#include <string>

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
    @param execution The task execution type flags.
    @param name      The string identifier of the task.

    @return The return type for task registration is determined by
            the specific backend runtime being used.
   */

  template<size_t KEY,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
  static decltype(auto)
  register_task(processor_type_t processor, task_execution_type_t execution,
			std::string name) {
    return EXECUTION_POLICY::
      template register_task<KEY, RETURN, ARG_TUPLE, DELEGATE>(
        processor, execution, name);
  } // register_task

  /*!
    Register a launch domain with the FleCSI runtime.

    @tparam KEY       A hash key identifying the domain.
    @tparam LAUNCH    The launch type (single, index).
    @tparam SIZE      A domain size
   */

  template<size_t KEY,
    launch_type_t LAUNCH,
    size_t SIZE>
  static decltype(auto)
  register_domain() {
    execution::context_t::instance().register_domain(KEY, LAUNCH, SIZE);
    return true;
  } // register_domain

  /*!
    Execute a task.

    @tparam LAUNCH    The launch mode for this task execution.
    @tparam TASK      The task hash key.
    @tparam REDUCTION The reduction operation hash key.
    @tparam RETURN    The return type of the task.
    @tparam ARG_TUPLE A std::tuple of the user task argument types.
    @tparam ARGS      The task arguments.

    @param domain_key A hash from the domain name
    @param args   The arguments to pass to the user task during execution.
   */

  template<size_t TASK,
    size_t REDUCTION,
    typename RETURN,
    typename ARG_TUPLE,
    typename... ARGS>
  static decltype(auto) execute_task(size_t domain_key,
				ARGS &&... args) {
    return EXECUTION_POLICY::
      template execute_task<TASK, REDUCTION, RETURN, ARG_TUPLE>(
				domain_key, std::forward<ARGS>(args)...);
  } // execute_task

  /*!
    Register a custom reduction operation.

    @tparam HASH A hash key identifying the operation.
    @tparam TYPE The user-defined operation type. The interface
                 for this type is prescribed.
   */

  template<size_t HASH, typename TYPE>
  static decltype(auto) register_reduction_operation() {
    using wrapper_t =
      typename EXECUTION_POLICY::template reduction_wrapper_u<HASH, TYPE>;

    return context_t::instance().register_reduction_operation(
      HASH, wrapper_t::registration_callback);
  } // register_reduction_operation

}; // struct task_interface_u

} // namespace execution
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_EXECUTION_POLICY used below.
//----------------------------------------------------------------------------//
#include <flecsi/runtime/execution_policy.h>

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

// template<typename RETURN, launch_type_t launch>
// using future_u = FLECSI_RUNTIME_EXECUTION_POLICY::future_u<RETURN, launch>;

} // namespace execution
} // namespace flecsi
