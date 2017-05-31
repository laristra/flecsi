/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

#include "flecsi/execution/serial/runtime_driver.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Implementation of FleCSI runtime driver task.
//----------------------------------------------------------------------------//

void serial_runtime_driver(int argc, char ** argv) {

#if defined FLECSI_ENABLE_SPECIALIZATION_DRIVER
  // Execute the specialization driver.
  specialization_driver(argc, argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_DRIVER

  // Execute the user driver.
  driver(argc, argv);

} // serial_runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
