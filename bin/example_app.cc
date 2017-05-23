/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! @file example_app.cc
//! @date Initial file creation: Aug 25, 2016
//----------------------------------------------------------------------------//

#include <flecsi.h>
#include <mpi.h>

#include "flecsi/execution/execution.h"

int main(int argc, char ** argv) {

  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  // If you fail this assertion, then your version of MPI
  // does not support calls from multiple threads and you
  // cannot use the GASNet MPI conduit
  if (provided < MPI_THREAD_MULTIPLE)
    printf("ERROR: Your implementation of MPI does not support "
           "MPI_THREAD_MULTIPLE which is required for use of the "
           "GASNet MPI conduit with the Legion-MPI Interop!\n");
  assert(provided == MPI_THREAD_MULTIPLE);

  // Call FleCSI runtime initialization
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

//  MPI_Finalize();

  return retval;
} // main


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
