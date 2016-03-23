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

#ifndef flecsi_task_h
#define flecsi_task_h

#include <type_traits>

#include "default_execution_policy.h"

/*!
 * \file task.h
 * \authors bergen
 * \date Initial file creation: Oct 19, 2015
 */

namespace flecsi
{
/*!
  \class execution_t task.h
  \brief execution_t provides...
*/
template <typename execution_policy_t = default_execution_policy_t>
class execution_t : public execution_policy_t
{
 public:
  using return_type_t = typename execution_policy_t::return_type_t;

  // FIXME We may need task registration

  template <typename T, typename... Args>
  static return_type_t execute_driver(T && task, Args &&... args)
  {
    // Make sure that the task definition returns the correct type
//    static_assert(std::is_same<return_type_t,
//                      decltype(task(std::forward<Args>(args)...))>(),
//        "Driver must return flecsi::execution_t::return_type_t");

    // pass the driver to the execution policy for handling
    return execution_policy_t::execute_driver(
        std::forward<T>(task), std::forward<Args>(args)...);
  } // execute_driver

  template <typename T, typename... Args>
  static return_type_t execute_task(T && task, Args &&... args)
  {
    // Make sure that the task definition returns the correct type
    static_assert(std::is_same<return_type_t,
                      decltype(task(std::forward<Args>(args)...))>(),
        "Tasks must return flecsi::execution_t::return_type_t");

    // pass the task to the execution policy for handling
    return execution_policy_t::execute_task(
        std::forward<T>(task), std::forward<Args>(args)...);
  } // execute_task

}; // class execution_t

} // namespace flecsi

#define task

#endif // flecsi_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
