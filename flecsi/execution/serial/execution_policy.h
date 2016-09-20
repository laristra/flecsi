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
  //
  ///
  template<
    typename R,
    typename ... As
  >
  static
  bool
  register_task(
    task_hash_key_t key
  )
  {
  } // register_task

  ///
  //
  ///
  template<
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
    auto t = std::make_tuple(args ...);
    return tuple_function(user_task, t);
  } // execute_task
  
  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  ///
  // This method registers a user function with the current
  // execution context.
  //
  // \param key The function identifier.
  // \param user_function A reference to the user function as a std::function.
  //
  // \return A boolean value indicating whether or not the function was
  //         successfully registered.
  ///
  template<
    typename R,
    typename ... As
  >
  static
  bool
  register_function(
    const const_string_t & key,
    std::function<R(As ...)> & user_function
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
    auto t = std::make_tuple(args ...);
    return handle(context_t::instance().function(handle.key), t);
  } // execute_function

}; // struct serial_execution_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_serial_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
