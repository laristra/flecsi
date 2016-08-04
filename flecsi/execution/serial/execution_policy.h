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

#ifndef flecsi_serial_execution_policy_h
#define flecsi_serial_execution_policy_h

#include <tuple>

#include "flecsi/execution/context.h"
#include "flecsi/execution/processor.h"
#include "flecsi/utils/tuple_function.h"
#include "flecsi/execution/serial/runtime_driver.h"

/*!
 * \file serial/execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flecsi {
namespace execution {

/*!
  \struct serial_execution_policy serial_execution_policy.h
  \brief serial_execution_policy provides...
 */
struct serial_execution_policy_t
{
  using task_key_t = uintptr_t;

  template<typename R, typename ... As>
  static bool register_task(uintptr_t key, processor_t processor) {
  } // register_task

  template<typename T, typename ... As>
  static decltype(auto) execute_task(uintptr_t key, processor_t processor, T user_task, As ... args)
  {
    auto t = std::make_tuple(args ...);
    return tuple_function(user_task, t);
  } // execute_task
  
}; // struct serial_execution_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_serial_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
