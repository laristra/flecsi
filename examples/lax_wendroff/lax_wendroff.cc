/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <flecsi.h>
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi
  #include <mpi.h>
  #include <legion.h>
#endif

#include "flecsi/execution/execution.h"

///
// \file example_app.cc
// \authors bergen
// \date Initial file creation: Aug 25, 2016
///

int main(int argc, char ** argv) {

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#ifdef GASNET_CONDUIT_MPI
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
#else
   MPI_Init(&argc, &argv);
#endif


#endif

  // Call FleCSI runtime initialization
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif 

#endif

  return retval;
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
