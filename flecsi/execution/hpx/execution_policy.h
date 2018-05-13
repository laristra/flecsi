/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_hpx_execution_policy_h
#define flecsi_execution_hpx_execution_policy_h

#include <hpx/include/async.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/thread_executors.hpp>
#include <hpx/include/parallel_execution.hpp>

#include <functional>
#include <tuple>
#include <unordered_map>
#include <utility>

#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/hpx/future.h>
#include <flecsi/execution/hpx/runtime_driver.h>
#include <flecsi/execution/hpx/task_wrapper.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/export_definitions.h>
#include <flecsi/utils/tuple_function.h>
//#include "flecsi/execution/task.h"

///
// \file hpx/execution_policy.h
// \authors bergen
// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Future.
//----------------------------------------------------------------------------//

///
/// Executor interface.
///
template<typename RETURN, typename ARG_TUPLE>
struct executor__ {
  ///
  ///
  ///
  template<typename Exec, typename T, typename A>
  static hpx_future__<RETURN, launch_type_t::single>
  execute(Exec && exec, T fun, A && targs) {
    auto user_fun = (reinterpret_cast<RETURN (*)(ARG_TUPLE)>(fun));
    return hpx::async(
        std::forward<Exec>(exec), std::move(user_fun), std::forward<A>(targs));
  } // execute_task
}; // struct executor__

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

///
/// \struct hpx_execution_policy hpx_execution_policy.h
/// \brief hpx_execution_policy provides...
///
struct FLECSI_EXPORT hpx_execution_policy_t {

  template<typename R, launch_type_t launch = launch_type_t::single>
  using future__ = hpx_future__<R, launch>;

  //--------------------------------------------------------------------------//
  //! The task_wrapper__ type FIXME
  //!
  //! @tparam RETURN The return type of the task. FIXME
  //--------------------------------------------------------------------------//

  template<typename FUNCTOR_TYPE>
  using functor_task_wrapper__ =
      typename flecsi::execution::functor_task_wrapper__<FUNCTOR_TYPE>;

  struct runtime_state_t {};

  //   static
  //   runtime_state_t &
  //   runtime_state(
  //     void * task
  //   )
  //   {
  //     return {};
  //   }
  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  ///
  /// hpx task registration.
  ///
  /// \tparam R The return type of the task.
  /// \tparam A The arguments type of the task. This is a std::tuple of the
  ///           user task arguments.
  ///
  template<
    size_t KEY,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)
  >
  static
  bool
  register_task(
     processor_type_t processor,
     launch_t launch,
     std::string name
  )
  {
    return context_t::instance().template register_task<
      KEY, RETURN, ARG_TUPLE, DELEGATE>(processor, launch, name);
  } // register_task

  ///
  /// \tparam R The task return type.
  /// \tparam T The user task type.
  /// \tparam As The user task argument types.
  ///
  /// \param key
  /// \param user_task_handle
  /// \param args
  ///
  template<launch_type_t launch,size_t KEY, typename RETURN,
    typename ARG_TUPLE, typename... ARGS>
  static decltype(auto) execute_task(ARGS &&... args) {
    context_t & context_ = context_t::instance();

    // Get the function and processor type.
    auto fun = context_.task<KEY>();

    auto processor_type = context_.processor_type<KEY>();
    if (processor_type == processor_type_t::mpi)
    {
      {
        clog_tag_guard(execution);
        clog(info) << "Executing MPI task: " << KEY << std::endl;
      }

      return executor__<RETURN, ARG_TUPLE>::execute(
          context_t::instance().get_mpi_executor(),
          std::move(fun), std::make_tuple(std::forward<ARGS>(args)...));
    }

    return executor__<RETURN, ARG_TUPLE>::execute(
        context_t::instance().get_default_executor(),
        std::move(fun), std::make_tuple(std::forward<ARGS>(args)...));
  } // execute_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//
  template<
    size_t KEY,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*FUNCTION)(ARG_TUPLE)>
  static
  bool
  register_function()
  {
    return context_t::instance()
        .template register_function<KEY, RETURN, ARG_TUPLE, FUNCTION>();
  } // register_function

  ///
  /// This method looks up a function from the \e handle argument
  /// and executes the associated it with the provided \e args arguments.
  ///
  /// \param handle The function handle to execute.
  /// \param args A variadic argument list of the function parameters.
  ///
  /// \return The return type of the provided function handle.
  ///
  template<typename FUNCTION_HANDLE, typename... ARGS>
  static decltype(auto)
  execute_function(FUNCTION_HANDLE & handle, ARGS &&... args) {
    return handle(
        context_t::instance().function(handle.get_key()),
        std::make_tuple(std::forward<ARGS>(args)...));
  } // execute_function

}; // struct hpx_execution_policy_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_hpx_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
