/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#ifdef ENABLE_MPI
  #include <mpi.h>
#endif

#include <flecsi/execution/context.h>

//----------------------------------------------------------------------------//
//! FleCSI runtime main function.
//----------------------------------------------------------------------------//

int main(int argc, char ** argv) {

#if defined(ENABLE_MPI)
  // Get the MPI version
  int version, subversion;
  MPI_Get_version(&version, &subversion);

#if defined(GASNET_CONDUIT_MPI)
  if(version==3 && subversion>0) {
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
  }
  else {
    // Initialize the MPI runtime
    MPI_Init(&argc, &argv);
  } // if
#else
   MPI_Init(&argc, &argv);
#endif
#endif // ENABLE_MPI

  // Execute the flecsi runtime.
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

#if defined(ENABLE_MPI)
  // FIXME: This is some kind of GASNet bug (or maybe Legion).
  // Shutdown the MPI runtime
#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif
#endif // ENABLE_MPI

  return retval;
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
