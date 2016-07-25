/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef default_driver_h
#define default_driver_h

#include <iostream>

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {

void driver(int argc, char ** argv) {
  std::cout << "Executing default driver" << std::endl;
} // driver

} // namespace flecsi

#endif // default_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
