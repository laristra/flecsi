/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_task_driver_h
#define flecsi_task_driver_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Task registration.
//----------------------------------------------------------------------------//

double task(double dval, int ival) {
  std::cout << "Executing task" << std::endl;
  std::cout << "Value(double): " << dval << std::endl;
  std::cout << "Value(int): " << ival << std::endl;
  return dval;
} // task1

flecsi_register_task(task, loc | toc, single | index);

//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  const double alpha{10.0};

  auto f = flecsi_execute_task(task, loc, single, alpha, 5);

  //f.wait();

  //clog(info) << "Task return: " << f.get() << std::endl;

} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_task_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
