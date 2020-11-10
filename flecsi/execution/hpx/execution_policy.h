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

#include <hpx/include/async.hpp>
#include <hpx/include/lcos.hpp>

#include <cinchlog.h>
#include <functional>
#include <tuple>
#include <unordered_map>
#include <utility>

#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/hpx/finalize_handles.h>
#include <flecsi/execution/hpx/future.h>
#include <flecsi/execution/hpx/reduction_wrapper.h>
#include <flecsi/execution/hpx/task_epilog.h>
#include <flecsi/execution/hpx/task_prolog.h>
#include <flecsi/utils/annotation.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/export_definitions.h>
#include <flecsi/utils/tuple_type_converter.h>

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
    auto user_fun = reinterpret_cast<RETURN (*)(ARG_TUPLE)>(function);
    return hpx::make_ready_future(
      user_fun(utils::forward_tuple(std::forward<A>(targs))));
  } // execute_task
}; // struct executor_u

template<typename ARG_TUPLE>
struct executor_u<void, ARG_TUPLE> {
  /*!
   FIXME documentation
   */
  template<typename T, typename A>
  static decltype(auto) execute(T function, A && targs) {
    auto user_fun = reinterpret_cast<void (*)(ARG_TUPLE)>(function);
    user_fun(utils::forward_tuple(std::forward<A>(targs)));
    return hpx::make_ready_future();
  } // execute_task
}; // struct executor_u

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

/*!
  The hpx_execution_policy_t is the backend runtime execution policy
  for HPX.

  @ingroup hpx-execution
 */

struct FLECSI_EXPORT hpx_execution_policy_t {

  /*!
    The future_u type may be used for explicit synchronization of tasks.

    @tparam RETURN The return type of the task.
   */

  template<typename R, launch_type_t launch = launch_type_t::single>
  using future_u = hpx_future_u<R, launch>;

  /*!
    The runtime_state_t type identifies a public type for the high-level
    runtime interface to pass state required by the backend.
   */

  struct runtime_state_t {};
  // using runtime_state_t = hpx_runtime_state_t;

  /*!
    Return the runtime state of the calling FleCSI task.

    @param task The calling task.
   */

  static runtime_state_t & runtime_state(void * task);

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /*!
    HPX backend task registration. For documentation on this
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
    HPX backend task execution. For documentation on this method,
    please see task_u::execute_task.
   */

  template<launch_type_t launch,
    size_t TASK,
    size_t REDUCTION,
    typename RETURN,
    typename ARG_TUPLE,
    typename... ARGS>
  static decltype(auto) execute_task(ARGS &&... args) {

    // Make a tuple from the task arguments.
    using decayed_args = utils::convert_tuple_t<ARG_TUPLE, std::decay_t>;

    auto func = [function = context_t::instance().function(TASK),
                  task_args = decayed_args(
                    std::make_tuple(std::forward<ARGS>(args)...))]() mutable {
      context_t & context_ = context_t::instance();

      using annotation = flecsi::utils::annotation;
#if defined(ENABLE_CALIPER)
      auto tname = context_.function_name(TASK);
#else
      /* using a placeholder so we do not have to maintain
         function_name_registry when annotations are disabled. */
      std::string tname{""};
#endif

      annotation::begin<annotation::execute_task_prolog>(tname);
      // run task_prolog to copy ghost cells.
      task_prolog_t task_prolog;
      task_prolog.walk(task_args);
#if defined(FLECSI_USE_AGGCOMM)
      task_prolog.launch_copies();
      task_prolog.launch_sparse_copies();
#endif
      annotation::end<annotation::execute_task_prolog>();

      annotation::begin<annotation::execute_task_user>(tname);

      auto user_fun = reinterpret_cast<RETURN (*)(ARG_TUPLE)>(function);
      hpx_future_u<RETURN> future =
        executor_u<RETURN, ARG_TUPLE>::execute(function, task_args);
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

        return future
          .then([](hpx_future_u<RETURN> && future) {
            context_t & context_ = context_t::instance();
            MPI_Datatype datatype =
              flecsi::utils::mpi_typetraits_u<RETURN>::type();

            auto reduction_op = context_.reduction_operations().find(REDUCTION);

            clog_assert(reduction_op != context_.reduction_operations().end(),
              "invalid reduction operation");

            const RETURN sendbuf = future.get();
            RETURN recvbuf;

            MPI_Allreduce(&sendbuf, &recvbuf, 1, datatype, reduction_op->second,
              MPI_COMM_WORLD);

            return recvbuf;
          })
          .share();
      }

      return future;
    };

    // for unwrapping o returned future
    hpx_future_u<RETURN> future = hpx::async(std::move(func));
    future.wait();
    return future;
  } // execute_task

  //--------------------------------------------------------------------------//
  // Reduction interface.
  //--------------------------------------------------------------------------//

  /*!
    HPX backend reduction registration. For documentation on this
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
    HPX backend function registration. For documentation on this
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
    HPX backend function execution. For documentation on this
    method, please see function_u::execute_function.
   */

  template<typename HANDLE, typename... ARGS>
  static decltype(auto) execute_function(HANDLE & handle, ARGS &&... args) {
    return handle(context_t::instance().function(handle.get_key()),
      std::forward_as_tuple(args...));
  } // execute_function

}; // struct hpx_execution_policy_t

} // namespace execution
} // namespace flecsi
