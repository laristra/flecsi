/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef default_driver_h
#define default_driver_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/task.h"
//#include "flecsi/execution/legion/legion_execution_policy.h"

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

#define register_task(task, return_type, ...) \
  using user_delegate_ ## task ## _t = \
    std::function<return_type(__VA_ARGS__)>; \
  bool task ## _registered = \
    task_t::register_task<return_type, __VA_ARGS__>( \
    reinterpret_cast<uintptr_t>(&task));

#define execute_task(task, ...) \
  user_delegate_ ## task ## _t task ## _delegate = task; \
  task_t::execute_task(reinterpret_cast<uintptr_t>(&task), \
    task ## _delegate, ## __VA_ARGS__)

namespace flecsi {

void hello(double value) {
  std::cout << "Executing hello task" << std::endl;
  std::cout << "Value: " << value << std::endl;
} // hello

register_task(hello, void, double);

void driver(int argc, char ** argv) {

  execute_task(hello, 10.0);

} // driver

} // namespace flecsi

#endif // default_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
