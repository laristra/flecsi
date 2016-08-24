/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_sprint_h
#define flecsi_sprint_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"

///
// \file sprint.h
// \authors bergen
// \date Initial file creation: Aug 23, 2016
///

namespace flecsi {
namespace execution {

void mpi_task(double val) {
  int rank = 0;
//  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << "My rank: " << rank << std::endl;
} // mpi_task

register_task(mpi_task, mpi, void, double);

void driver(int argc, char ** argv) {
  execute_task(mpi_task, mpi, 1.0);
} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_sprint_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
