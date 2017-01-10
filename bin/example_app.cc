/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <flecsi.h>
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi
  #include <mpi.h>
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
  MPI_Init(&argc, &argv);
#endif

  // Call FleCSI runtime initialization
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi
  MPI_Finalize();
#endif

  return retval;
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
