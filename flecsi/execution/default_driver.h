/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_default_driver_h
#define flecsi_default_driver_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {

void task1(double dval, int ival) {
  std::cout << "Executing task1" << std::endl;
  std::cout << "Value(double): " << dval << std::endl;
  std::cout << "Value(int): " << ival << std::endl;
} // task1

register_task(task1, void, double, int);

double task2(double x, double y) {
  std::cout << "Executing task2" << std::endl;
  std::cout << "(x,y): (" << x << "," << y << ")" << std::endl;
  std::cout << "Return: " << x*y << std::endl;
//  return x*y;
} // task2

register_task(task2, void, double, double);

void driver(int argc, char ** argv) {

  double alpha(10.0);

  execute_task(task1, alpha, 5);
  execute_task(task2, alpha, 5.0);

} // driver

} // namespace flecsi

#endif // flecsi_default_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
