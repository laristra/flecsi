/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include <cinchlog.h>

#include "flecsi/execution/execution.h"

namespace flecsi {
namespace execution {

void mpi_task(double val) {
  clog(info) << "Hello World " << val << std::endl;
} // mpi_task

flecsi_register_task(mpi_task, mpi, single);

void specialization_driver(int argc, char ** argv) {
  clog(info) << "In specialization driver" << std::endl;

  double alpha(2.0);
  flecsi_execute_task(mpi_task, mpi, single, alpha);

} // specialization_driver

void driver(int argc, char ** argv) {
  clog(info) << "In driver" << std::endl;
} // specialization_driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
