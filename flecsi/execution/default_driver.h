/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_default_driver_h
#define flecsi_default_driver_h

#include <iostream>

//#include "flecsi/utils/common.h"
//#include "flecsi/execution/context.h"
//#include "flecsi/execution/execution.h"
//#include "flecsi/data/data.h"

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {
namespace execution {

/*----------------------------------------------------------------------------*
 * Default driver.
 *----------------------------------------------------------------------------*/

void driver(int argc, char ** argv) {

  std::cout << "Warning: You are executing the default driver!" << std::endl;
  std::cout <<
    "This driver has no functionality except as a place holder..." <<
    std::endl;

} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_default_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
