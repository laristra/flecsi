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
#include <flecsi/execution/common/launch.hh>
#include <flecsi/execution/common/processor.hh>
#endif

#include"../utils/demangle.hh"  // symbol
// We can't use internal.hh; it depends on us.
#include"../utils/function_traits.hh"

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
  /// An arbitrary index for each task function.
  /// \tparam F task function
  /// \tparam P processor type
  /// \tparam L launch type
  template<auto &F,processor_type_t P,launch_type_t L>
  static const std::size_t task_id;

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
  static decltype(auto) register_task(processor_type_t processor,
    task_execution_type_t execution,
    std::string name) {
    return EXECUTION_POLICY::
      template register_task<RETURN, ARG_TUPLE, DELEGATE>
        (KEY, processor, execution, name);
  } // register_task

  /*!
    Register a launch domain with the FleCSI runtime.

    @tparam KEY       A hash key identifying the domain.
    @tparam SIZE      A domain size
   */

  template<size_t KEY, size_t SIZE>
  static decltype(auto) register_domain() {
    execution::context_t::instance().register_index_domain(KEY, SIZE);
    return true;
  } // register_domain

  /*!
    Execute a task.

    @tparam LAUNCH    The launch mode for this task execution.
    @tparam REDUCTION The reduction operation hash key.
    @tparam RETURN    The return type of the task.
    @tparam ARG_TUPLE A std::tuple of the user task argument types.
    @tparam ARGS      The task arguments.

    @param TASK      The task hash key.
    @param args   The arguments to pass to the user task during execution.
   */

  template<
    size_t LAUNCH_DOMAIN,
    size_t REDUCTION,
    typename RETURN,
    typename ARG_TUPLE,
    typename... ARGS>
  static decltype(auto) execute_task(std::size_t TASK,ARGS &&... args) {
    return EXECUTION_POLICY::
      template execute_task<LAUNCH_DOMAIN, REDUCTION, RETURN, ARG_TUPLE>(TASK,
        std::forward<ARGS>(args)...);
  } // execute_task

  /// Execute a task.
  /// \tparam F task function
  /// \tparam P processor type
  /// \tparam L launch type
  /// \tparam Dom launch domain
  /// \tparam Args task argument types
  /// \param args task arguments
  template<auto &F,processor_type_t P=loc,launch_type_t L=index,
           std::size_t Dom=flecsi_internal_hash(single),
           typename ...Args>
  static decltype(auto) execute(Args &&...args) {
    using Traits=utils::function_traits_u<decltype(F)>;
    return EXECUTION_POLICY::template execute_task
      <Dom,flecsi_internal_hash(0),typename Traits::return_type,
       typename Traits::arguments_type>
      (task_id<F,P,L>,std::forward<Args>(args)...);
  }

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

private:
  template<auto &F>
  static std::size_t register_task(processor_type_t p,
                                   task_execution_type_t e) {
    using Traits=utils::function_traits_u<decltype(F)>;
    using Args=typename Traits::arguments_type;
    constexpr auto delegate=+[](Args a) {return std::apply(F,a);};
    EXECUTION_POLICY::template register_task
      <typename Traits::return_type,Args,delegate>
      (next_task,p,e,utils::symbol<F>());
    return next_task++;
  }

  static inline std::size_t next_task=0;
}; // struct task_interface_u

template<class E> template<auto &F,processor_type_t P,launch_type_t L>
const std::size_t task_interface_u<E>::task_id=register_task<F>(P,L);

} // namespace execution
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_EXECUTION_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/execution_policy.hh>

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
