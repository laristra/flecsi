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

#ifndef flecsi_execution_serial_execution_policy_h
#define flecsi_execution_serial_execution_policy_h

#include <tuple>
#include <functional>
#include <unordered_map>
#include <future>

#include "flecsi/execution/context.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/launch.h"
#include "flecsi/utils/tuple_function.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/execution/serial/task_wrapper.h"
#include "flecsi/execution/serial/runtime_driver.h"
//#include "flecsi/execution/task.h"


///
// \file serial/execution_policy.h
// \authors bergen
// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Future.
//----------------------------------------------------------------------------//

///
/// implementation of the future_t for serial runtime
///
template<
  typename R
>
struct serial_future__
{
  using result_t = R;

  ///
  /// wait() method 
  ///
  void wait() {}

  ///
  /// get() mothod
  ///
  const result_t & get(size_t index = 0) const { return result_; }

//private:

  ///
  /// set method
  /// 
  void set(const result_t & result) { result_ = result; }

  result_t result_;

}; // struct serial_future__

///
///
///
template<>
struct serial_future__<void>
{
  ///
  ///
  ///
  void wait() {}

}; // struct serial_future__

///
/// Executor interface.
///
template<
  typename RETURN,
  typename ARG_TUPLE>
struct executor__
{
  ///
  ///
  ///
  template<
    typename T,
    typename A
  >
  static
  decltype(auto)
  execute(
    T fun,
    A && targs
  )
  {
    auto user_fun = (reinterpret_cast<RETURN(*)(ARG_TUPLE)>(fun));
    serial_future__<RETURN> fut;
    fut.set(user_fun(targs));
    return fut;
  } // execute_task
}; // struct executor__

template<
  typename ARG_TUPLE
>
struct executor__<void, ARG_TUPLE>
{
  ///
  ///
  ///
  template<
    typename T,
    typename A
  >
  static
  decltype(auto)
  execute(
    T fun,
    A && targs
  )
  {
    auto user_fun = (reinterpret_cast<void(*)(ARG_TUPLE)>(fun));

    serial_future__<void> fut;
    user_fun(targs);

    return fut;
  } // execute_task
}; // struct executor__

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

///
/// \struct serial_execution_policy serial_execution_policy.h
/// \brief serial_execution_policy provides...
///
struct serial_execution_policy_t
{
  template<typename R>
  using future__ = serial_future__<R>;

  //--------------------------------------------------------------------------//
  //! The task_wrapper__ type FIXME
  //!
  //! @tparam RETURN The return type of the task. FIXME
  //--------------------------------------------------------------------------//

  template<
    typename FUNCTOR_TYPE
  >
  using functor_task_wrapper__ =
    typename flecsi::execution::functor_task_wrapper__<FUNCTOR_TYPE>;

  struct runtime_state_t {};
  static
  runtime_state_t &
  runtime_state(
    void * task
  );
  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  ///
  /// Serial task registration.
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
    return context_t::instance().template register_function<
      RETURN, ARG_TUPLE, DELEGATE, KEY>();
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
  template<
    size_t KEY,
    typename RETURN,
    typename ARG_TUPLE,
    typename ... ARGS
  >
  static
  decltype(auto)
  execute_task(
    launch_type_t launch,
    size_t parent,
    ARGS && ... args
  )
  {
    auto fun = context_t::instance().function(KEY);
    return executor__<RETURN, ARG_TUPLE>::execute(fun, std::forward_as_tuple(args ...));
  } // execute_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//
  template<
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*FUNCTION)(ARG_TUPLE),
    size_t KEY
  >
  static
  bool
  register_function()
  {
    return context_t::instance().template register_function<
      RETURN, ARG_TUPLE, FUNCTION, KEY>();
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
  template<
    typename FUNCTION_HANDLE,
    typename ... ARGS
  >
  static
  decltype(auto)
  execute_function(
    FUNCTION_HANDLE & handle,
    ARGS && ... args
  )
  {
    return handle(context_t::instance().function(handle.key()),
      std::forward_as_tuple(args ...));
  } // execute_function

}; // struct serial_execution_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_serial_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
