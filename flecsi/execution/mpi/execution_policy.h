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

#include <cinchlog.h>
#include <functional>
#include <future>
#include <memory>
#include <type_traits>

#include "flecsi/utils/tuple_type_converter.h"
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/mpi/finalize_handles.h>
#include <flecsi/execution/mpi/future.h>
#include <flecsi/execution/mpi/reduction_wrapper.h>
#include <flecsi/execution/mpi/task_epilog.h>
#include <flecsi/execution/mpi/task_prolog.h>
#include <flecsi/utils/annotation.h>

namespace flecsi {
namespace execution {

/*!
  Executor interface.
 */

template<typename RETURN, typename ARG_TUPLE>
struct executor_u {
  /*!
   FIXME documentation
   */
  template<typename T, typename A>
  static decltype(auto) execute(T function, A && targs) {

    auto user_fun = (reinterpret_cast<RETURN (*)(ARG_TUPLE)>(function));
    mpi_future_u<RETURN> future;
    future.set(user_fun(utils::forward_tuple(std::forward<A>(targs))));

    return future;
  } // execute
}; // struct executor_u

/*!
 FIXME documentation
 */

template<typename ARG_TUPLE>
struct executor_u<void, ARG_TUPLE> {
  /*!
   FIXME documentation
   */
  template<typename T, typename A>
  static decltype(auto) execute(T function, A && targs) {

    auto user_fun = (reinterpret_cast<void (*)(ARG_TUPLE)>(function));

    mpi_future_u<void> future;
    user_fun(utils::forward_tuple(std::forward<A>(targs)));

    return future;
  } // execute_task
}; // struct executor_u

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

/*!
  The mpi_execution_policy_t is the backend runtime execution policy
  for MPI.

  @ingroup mpi-execution
 */

struct mpi_execution_policy_t {

  /*!
    The future_u type may be used for explicit synchronization of tasks.

    @tparam RETURN The return type of the task.
   */

  template<typename RETURN, launch_type_t launch>
  using future_u = mpi_future_u<RETURN, launch>;

  /*!
    The runtime_state_t type identifies a public type for the high-level
    runtime interface to pass state required by the backend.
   */

  struct runtime_state_t {};
  // using runtime_state_t = mpi_runtime_state_t;

  /*!
    Return the runtime state of the calling FleCSI task.

    @param task The calling task.
   */

  static runtime_state_t & runtime_state(void * task);

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /*!
    MPI backend task registration. For documentation on this
    method please see task_u::register_task.
   */

  template<size_t TASK,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
  static bool
  register_task(processor_type_t processor, launch_t launch, std::string name) {
#if defined(ENABLE_CALIPER)
    return context_t::instance()
      .template register_function<TASK, RETURN, ARG_TUPLE, DELEGATE>(name);
#else
    return context_t::instance()
      .template register_function<TASK, RETURN, ARG_TUPLE, DELEGATE>();
#endif
  } // register_task

  /*!
    MPI backend task execution. For documentation on this method,
    please see task_u::execute_task.
   */

  template<launch_type_t launch,
    size_t TASK,
    size_t REDUCTION,
    typename RETURN,
    typename ARG_TUPLE,
    typename... ARGS>
  static decltype(auto) execute_task(ARGS &&... args) {

    context_t & context_ = context_t::instance();

    auto function = context_.function(TASK);

    using annotation = flecsi::utils::annotation;
#if defined(ENABLE_CALIPER)
    auto tname = context_.function_name(TASK);
#else
    /* using a placeholder so we do not have to maintain function_name_registry
       when annotations are disabled. */
    std::string tname{""};
#endif

    // Make a tuple from the task arguments.
    utils::convert_tuple_t<ARG_TUPLE, std::decay_t> task_args =
      std::make_tuple(std::forward<ARGS>(args)...);

    annotation::begin<annotation::execute_task_prolog>(tname);
    // run task_prolog to copy ghost cells.
    task_prolog_t task_prolog;
    task_prolog.walk(task_args);
#if defined(FLECSI_USE_AGGCOMM)
    task_prolog.launch_rowsize_exchange();
    /* with current sparse implementation we need row_resizer to handle any
       sparse row resizing (although this communication is aggregated).*/
    task_prolog.row_resizer.walk(task_args);

    task_prolog.launch_dense_exchange();
    task_prolog.launch_sparse_exchange();
#endif
    annotation::end<annotation::execute_task_prolog>();

    annotation::begin<annotation::execute_task_user>(tname);
    auto future = executor_u<RETURN, ARG_TUPLE>::execute(function, task_args);
    annotation::end<annotation::execute_task_user>();

    annotation::begin<annotation::execute_task_epilog>(tname);
    task_epilog_t task_epilog;
    task_epilog.walk(task_args);
    annotation::end<annotation::execute_task_epilog>();

    annotation::begin<annotation::execute_task_finalize>(tname);
    finalize_handles_t finalize_handles;
    finalize_handles.walk(task_args);
    annotation::end<annotation::execute_task_finalize>();

    constexpr size_t ZERO =
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(0)}.hash();

    if constexpr(REDUCTION != ZERO) {

      auto reduction_op = context_.reduction_operations().find(REDUCTION);

      clog_assert(reduction_op != context_.reduction_operations().end(),
        "invalid reduction operation");

      future.reduce(reduction_op->second);
      return future;
    }
    else {
      return future;
    } // if
  } // execute_task

  //--------------------------------------------------------------------------//
  // Reduction interface.
  //--------------------------------------------------------------------------//

  /*!
    MPI backend reduction registration. For documentation on this
    method please see task_u::register_reduction_operation.
   */

  template<size_t NAME, typename OPERATION>
  static bool register_reduction_operation() {
    using wrapper_t = reduction_wrapper_u<NAME, OPERATION>;

    return context_t::instance().register_reduction_operation(
      NAME, wrapper_t::registration_callback);
  } // register_reduction_operation

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  /*!
    MPI backend function registration. For documentation on this
    method, please see function_u::register_function.
   */

  template<size_t FUNCTION,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
  static bool register_function() {
    return context_t::instance()
      .template register_function<FUNCTION, RETURN, ARG_TUPLE, DELEGATE>();
  } // register_function

  /*!
    MPI backend function execution. For documentation on this
    method, please see function_u::execute_function.
   */

  template<typename HANDLE, typename... ARGS>
  static decltype(auto) execute_function(HANDLE & handle, ARGS &&... args) {
    return handle(context_t::instance().function(handle.get_key()),
      std::forward_as_tuple(args...));
  } // execute_function

}; // struct mpi_execution_policy_t

} // namespace execution
} // namespace flecsi
