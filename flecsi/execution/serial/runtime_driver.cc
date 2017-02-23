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

#ifndef FLECSI_DRIVER
  #include "flecsi/execution/default_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_DRIVER)
#endif

#ifndef FLECSI_SPECIALIZATION_DRIVER
  #include "flecsi/execution/default_specialization_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_SPECIALIZATION_DRIVER)
#endif

namespace flecsi {
namespace execution {

void serial_runtime_driver(int argc, char ** argv) {

  // run default or user-defined specialization driver  
  specialization_driver(argc, argv);
  // run default or user-defined driver
  driver(argc, argv);

} // serial_runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
