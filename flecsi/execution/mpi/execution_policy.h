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

#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/mpi/finalize_handles.h>
#include <flecsi/execution/mpi/future.h>
#include <flecsi/execution/mpi/reduction_wrapper.h>
#include <flecsi/execution/mpi/task_epilog.h>
#include <flecsi/execution/mpi/task_prolog.h>


#if defined(ENABLE_CALIPER)
#include<caliper/Annotation.h>
#endif

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
    future.set(user_fun(std::forward<A>(targs)));

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
    user_fun(std::forward<A>(targs));

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

    // Make a tuple from the task arguments.
    ARG_TUPLE task_args = std::make_tuple(std::forward<ARGS>(args)...);

    // run task_prolog to copy ghost cells.
    task_prolog_t task_prolog;
    task_prolog.walk(task_args);

#if defined(ENABLE_CALIPER)
    cali::Annotation ep("FleCSI-Execution");
    auto tname = context_.function_name(TASK);
    std::string atag = "execute_task->user->" + tname;
    ep.begin(atag.c_str());
#endif
    auto future = executor_u<RETURN, ARG_TUPLE>::execute(function, task_args);
#if defined(ENABLE_CALIPER)
    ep.end();
#endif

    task_epilog_t task_epilog;
    task_epilog.walk(task_args);

#if defined(ENABLE_CALIPER)
    atag = "execute_task->finalize-handles->" + tname;
    ep.begin(atag.c_str());
#endif
    finalize_handles_t finalize_handles;
    finalize_handles.walk(task_args);
#if defined(ENABLE_CALIPER)
    ep.end();
#endif


    constexpr size_t ZERO =
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(0)}.hash();

    if constexpr(REDUCTION != ZERO) {

      MPI_Datatype datatype;

      if constexpr(!std::is_pod_v<RETURN>) {

        size_t typehash = typeid(RETURN).hash_code();
        auto reduction_type = context_.reduction_types().find(typehash);

        clog_assert(reduction_type != context_.reduction_types().end(),
          "invalid reduction operation");

        datatype = reduction_type->second;
      }
      else {
        datatype = flecsi::utils::mpi_typetraits_u<RETURN>::type();
      } // if

      auto reduction_op = context_.reduction_operations().find(REDUCTION);

      clog_assert(reduction_op != context_.reduction_operations().end(),
        "invalid reduction operation");

      const RETURN sendbuf = future.get();
      RETURN recvbuf;

      MPI_Allreduce(
        &sendbuf, &recvbuf, 1, datatype, reduction_op->second, MPI_COMM_WORLD);

      mpi_future_u<RETURN> gfuture;
      gfuture.set(recvbuf);
      return gfuture;
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
