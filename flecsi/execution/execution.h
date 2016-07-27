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

#ifndef flecsi_execution_h
#define flecsi_execution_h

#include <type_traits>

#include "flecsi/execution/default_execution_policy.h"

/*!
 * \file execution.h
 * \authors bergen
 * \date Initial file creation: Oct 19, 2015
 */

namespace flecsi
{

/*!
  \class execution_ execution.h
  \brief execution_ provides...
*/
template <typename execution_policy_t = default_execution_policy_t>
class execution_ : public execution_policy_t
{
public:

  using return_type_t = typename execution_policy_t::return_type_t;

  static int initialize(int argc, char ** argv)
  {
    return execution_policy_t::initialize(argc, argv);
  } // initialize

  template <typename T, typename... Args>
  static return_type_t execute_driver(T && task, Args &&... args)
  {
    return execution_policy_t::execute_driver(
        std::forward<T>(task), std::forward<Args>(args)...);
  } // execute_driver

  template <typename T, typename... Args>
  static return_type_t execute_task(T && task, Args &&... args)
  {
#if 0
    // Make sure that the task definition returns the correct type
    static_assert(std::is_same<return_type_t,
                      decltype(task(std::forward<Args>(args)...))>(),
        "Tasks must return flecsi::execution_::return_type_t");
#endif
    return execution_policy_t::execute_task(
        std::forward<T>(task), std::forward<Args>(args)...);
  } // execute_task

}; // class execution_

} // namespace flecsi

#endif // flecsi_execution_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
