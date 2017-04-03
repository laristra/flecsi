/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
// \file serial/runtime_driver.cc
// \authors bergen
// \date Initial file creation: Aug 01, 2016
///

#include "flecsi/execution/serial/runtime_driver.h"
#include "flecsi/utils/common.h"

namespace flecsi {
namespace execution {

// driver prototype
void serial_runtime_driver(int argc, char ** argv) {

#ifndef FLECSI_OVERRIDE_DEFAULT_SPECIALIZATION_DRIVER

  // run user-defined specialization driver  
  specialization_driver(argc, argv);

#endif // FLECSI_OVERRIDE_DEFAULT_SPECIALIZATION_DRIVER

  // run default or user-defined driver
  driver(argc, argv);

} // serial_runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
