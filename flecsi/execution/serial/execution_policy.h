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

// Forward.
template<typename R> struct executor__;

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
  friend executor__<R>;
  using result_t = R;

  ///
  /// wait() method 
  ///
  void wait() {}

  ///
  /// get() mothod
  ///
  const result_t & get(size_t index = 0) const { return result_; }

private:  

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
  friend executor__<void>;

  ///
  ///
  ///
  void wait() {}

}; // struct serial_future__

//----------------------------------------------------------------------------//
// Executor.
//----------------------------------------------------------------------------//

///
/// Executor interface.
///
template<
  typename R
>
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
    task_hash_key_t key,
    size_t parent,
    T user_task_handle,
    A && targs
  )
  {
    R value =
      user_task_handle(context_t::instance().function(user_task_handle.key),
        std::forward<A>(targs));
    serial_future__<R> f;
    f.set(value);
    return f;
  } // execute_task
}; // struct executor__

///
/// Partial specialization for reference type.
///
template<
  typename R
>
struct executor__<R &>
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
    task_hash_key_t key,
    size_t parent,
    T user_task_handle,
    A && targs
  )
  {
    R & value =
      user_task_handle(context_t::instance().function(user_task_handle.key),
        std::forward<A>(targs));
    serial_future__<R &> f;
    f.set(value);
    return f;
  } // execute_task
}; // struct executor__

///
/// Explicit specialization for void
///
template<>
struct executor__<void>
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
    task_hash_key_t key,
    size_t parent,
    T user_task_handle,
    A targs
  )
  {
    user_task_handle(context_t::instance().function(user_task_handle.key),
      targs);
    return serial_future__<void>();
  } // execute

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
    typename R,
    typename A
  >
  static
  bool
  register_task(
    task_hash_key_t key
  )
  {
    return false;
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
    typename R,
    typename T,
    typename...As
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    size_t parent,
    T user_task_handle,
    As && ... args
  )
  {
    return executor__<R>::execute(
      key, parent, user_task_handle, std::forward_as_tuple(args...)
    );
  } // execute_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  ///
  /// This method registers a user function with the current
  /// execution context.
  ///
  /// \tparam R Return type.
  /// \tparam A Argument type (std::tuple).
  ///
  /// \param key The function identifier.
  /// \param user_function A reference to the user function as a std::function.
  ///
  /// \return A boolean value indicating whether or not the function was
  ///         successfully registered.
  ///
  template<
    typename R,
    typename A
  >
  static
  bool
  register_function(
    const utils::const_string_t & key,
    std::function<R(A)> & user_function
  )
  {
    return context_t::instance().register_function(key, user_function);
  } // register_function

  ///
  /// This method looks up a function from the \e handle argument
  /// and executes the associated it with the provided \e args arguments.
  ///
  // \param handle The function handle to execute.
  /// \param args A variadic argument list of the function parameters.
  ///
  /// \return The return type of the provided function handle.
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
    auto targs = std::forward_as_tuple( std::forward<As>(args) ...);
    return handle(context_t::instance().function(handle.key), targs);
  } // execute_function

}; // struct serial_execution_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_serial_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
