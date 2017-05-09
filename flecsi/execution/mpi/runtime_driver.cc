/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! \file
//! \date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include "flecsi/execution/mpi/runtime_driver.h"

#include "flecsi/execution/context.h"

clog_register_tag(runtime_driver);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Implementation of FleCSI runtime driver task.
//----------------------------------------------------------------------------//

void
runtime_driver(
  int argc,
  char ** argv
)
{
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "In MPI runtime driver" << std::endl;
  }

#if defined FLECSI_ENABLE_SPECIALIZATION_DRIVER
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization driver task" << std::endl;
  }

  // Execute the specialization driver.
  specialization_driver(args.argc, args.argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_DRIVER

  // Execute the user driver.
  driver(argc, argv);

} // runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
