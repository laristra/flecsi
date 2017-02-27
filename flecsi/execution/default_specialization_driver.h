/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_default_specialization_driver_h
#define flecsi_execution_default_specialization_driver_h

#include <iostream>

///
/// \file default_specialization_driver.h
///

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Default driver.
//----------------------------------------------------------------------------//

void specialization_driver(int argc, char ** argv) {

  std::cout << "Warning: You are executing the default specialization driver!"
     << std::endl;
  std::cout <<
    "This driver has no functionality except as a place holder..." <<
    std::endl;

} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_default_specialization_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
