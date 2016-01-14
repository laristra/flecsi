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

#ifndef flecsi_default_execution_policy_h
#define flecsi_default_execution_policy_h

#include "flecsi/execution/context.h"
#include "flecsi/utils/tuple_for_each.h"

/*!
 * \file default_execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flecsi {

/*!
  \class default_execution_policy default_execution_policy.h
  \brief default_execution_policy provides...
 */
class default_execution_policy_t
{
protected:

  using return_type_t = int32_t;

  template<typename T, typename ... Args>
  static return_type_t execute_driver(T && task, Args && ... args) {
    return task(std::forward<Args>(args) ...);
  } // execute_driver

  template<typename T, typename ... Args>
  static return_type_t execute_task(T && task, Args && ... args) {

#if 0
    // FIXME: place-holder example of static argument processing
    utils::tuple_for_each(std::make_tuple(args ...), [&](auto arg) {
      std::cout << "test" << std::endl;
      });
#endif

    context_t::instance().entry();
    auto value = task(std::forward<Args>(args) ...);
    context_t::instance().exit();
    return value;
  } // execute_task
  
}; // class default_execution_policy_t

} // namespace flecsi

#endif // flecsi_default_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
