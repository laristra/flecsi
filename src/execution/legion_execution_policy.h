/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_legion_execution_policy_h
#define flexi_legion_execution_policy_h

#include <iostream>
#include <utility>

#include "flexi/execution/context.h"
#include "flexi/utils/tuple_for_each.h"

/*!
 * \file legion_execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flexi {

/*!
  \class legion_execution_policy legion_execution_policy.h
  \brief legion_execution_policy provides...
 */
class legion_execution_policy_t
{
protected:

  using return_type_t = int32_t;

  template<typename T, typename ... Args>
  static return_type_t execute_driver(T && task, Args && ... args) {
    return task(std::forward<Args>(args) ...);
  } // execute_driver

  template<typename T, typename ... Args>
  static return_type_t execute_task(T && task, Args && ... args) {

    utils::tuple_for_each(std::make_tuple(args ...), [&](auto arg) {
      std::cout << "test" << std::endl;
      });

    context_t::instance().entry();
    auto value = task(std::forward<Args>(args) ...);
    context_t::instance().exit();
    return value;
  } // execute_task

}; // class legion_execution_policy_t

} // namespace flexi

#endif // flexi_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
