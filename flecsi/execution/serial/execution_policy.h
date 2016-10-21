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

#include "flecsi/execution/context.h"
//#include "flecsi/execution/future.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/utils/tuple_function.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/execution/serial/runtime_driver.h"

///
// \file serial/execution_policy.h
// \authors bergen
// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

template<typename R> struct future__;

template<
  typename R,
  typename T,
  typename A,
  bool is_void = std::is_void<R>::value
>
struct executor__
{

  static
  decltype(auto)
  execute(
    task_hash_key_t key,
    T user_task,
    A targs
  )
  {
    user_task(targs);
    return future__<R>();
  } // execute

}; // struct executor__

template<
  typename R,
  typename T,
  typename A
>
struct executor__<R, T, A, false>
{
  static
  decltype(auto)
  execute(
    task_hash_key_t key,
    T user_task,
    A targs
  )
  {
    // FIXME: Need to set state
    R value = user_task(targs);
    future__<R> f;
    f.set(value);
    return f;
    //return future__<R>(user_task(targs));
  } // execute_task
}; // struct executor__

///
// \struct serial_execution_policy serial_execution_policy.h
// \brief serial_execution_policy provides...
///
struct serial_execution_policy_t
{
  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  // FIXME: Finish Doxygen

  ///
  // Serial task registration.
  //
  // \tparam R The return type of the task.
  // \tparam A The arguments type of the task. This is a std::tuple of the
  //           user task arguments.
  ///
  template<
    typename R,
    typename A
  >
  static
  bool
  register_task(
    task_hash_key_t key
  )
  {
  } // register_task

  ///
  // \tparam R The task return type.
  // \tparam T The user task type.
  // \tparam As The user task argument types.
  //
  // \param key
  // \param user_task
  // \param args
  ///
  template<
    typename R,
    typename T,
    typename ... As
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    T user_task,
    As ... args
  )
  {
    using executor_t = executor__<R, T, std::tuple<As ...>>;
    return executor_t::execute(key, user_task, std::make_tuple(args ...));
  } // execute_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  ///
  // This method registers a user function with the current
  // execution context.
  //
  // \tparam R Return type.
  // \tparam A Argument type (std::tuple).
  //
  // \param key The function identifier.
  // \param user_function A reference to the user function as a std::function.
  //
  // \return A boolean value indicating whether or not the function was
  //         successfully registered.
  ///
  template<
    typename R,
    typename A
  >
  static
  bool
  register_function(
    const const_string_t & key,
    std::function<R(A)> & user_function
  )
  {
    context_t::instance().register_function(key, user_function);
  } // register_function

  ///
  // This method looks up a function from the \e handle argument
  // and executes the associated it with the provided \e args arguments.
  //
  // \param handle The function handle to execute.
  // \param args A variadic argument list of the function parameters.
  //
  // \return The return type of the provided function handle.
  ///
  template<
    typename T,
    typename ... As
  >
  static
  decltype(auto)
  execute_function(
    T & handle,
    As && ... args
  )
  {
    auto targs = std::make_tuple(args ...);
    return handle(context_t::instance().function(handle.key), targs);
  } // execute_function

}; // struct serial_execution_policy_t

template<
  typename R
>
struct future__
{
  friend serial_execution_policy_t;
}; // struct future__

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_serial_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
