/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef default_driver_h
#define default_driver_h

#include <iostream>

#include "flecsi/execution/context.h"
//#include "flecsi/execution/task.h"
#include "flecsi/execution/legion/legion_execution_policy.h"

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {

void hello() {
  std::cout << "Executing hello task" << std::endl;
} // hello

void driver(int argc, char ** argv) {
  std::cout << "Executing default driver" << std::endl;
} // driver

} // namespace flecsi

#endif // default_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
