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

void task1(double dval, int ival) {
  std::cout << "Executing task1" << std::endl;
  std::cout << "Value(double): " << dval << std::endl;
  std::cout << "Value(int): " << ival << std::endl;
} // task1

register_task(task1, void, double, int);

void task2(double x, double y) {
  std::cout << "Executing task2" << std::endl;
  std::cout << "(x,y): (" << x << "," << y << ")" << std::endl;
  std::cout << "Return: " << x*y << std::endl;
//  return x*y;
} // task2

register_task(task2, void, double, double);

void driver(int argc, char ** argv) {

  double alpha(10.0);

  execute_task(task1, alpha, 5);
  execute_task(task2, alpha, 5);

} // driver

} // namespace flecsi

#endif // default_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
