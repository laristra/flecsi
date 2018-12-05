/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>
#include <flecsi/control/runtime.h>

#if !defined(FLECSI_ENABLE_MPI)
  #error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <cstdlib>

/*
  Initialization for Legion runtime.
  
  - Initialize the MPI runtime with thread-multiple
 */

inline int initialize(int argc, char ** argv) {
  std::cout << "Executing initialize" << std::endl;

#if defined(FLECSI_ENABLE_MPI)
  // Get the MPI version
  int version, subversion;
  MPI_Get_version(&version, &subversion);

#if defined(GASNET_CONDUIT_MPI)
  if(version==3 && subversion>0) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    if(provided < MPI_THREAD_MULTIPLE) {
      std::cerr <<
        "Your implementation of MPI does not support "
        "MPI_THREAD_MULTIPLE which is required for use of the "
        "GASNet MPI conduit with the Legion-MPI Interop!"
        << std::endl;
      std::abort();
    } // if
  }
  else {
    // Initialize the MPI runtime
    MPI_Init(&argc, &argv);
  } // if
#else
  MPI_Init(&argc, &argv);
#endif

  // get the rank
  int rank{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#endif // FLECSI_ENABLE_MPI

  return 0;
} // initialize

/*
  Finalization for Legion runtime.
  
  - Shutdown MPI
 */

inline int finalize(int argc, char ** argv, exit_mode_t mode) {
  std::cout << "Executing finalize with mode " << size_t{mode} << std::endl;

#if defined(FLECSI_ENABLE_MPI)
  // Shutdown the MPI runtime
#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif
#endif // FLECSI_ENABLE_MPI

  return 0;
} // initialize

inline bool output(int argc, char ** argv) {
  return false;
} // output

inline runtime_handle_t handler{ initialize, finalize, output };

flecsi_append_runtime_handler(handler);

inline int runtime_driver(int argc, char ** argv) {
  return 0;
} // runtime_driver

flecsi_register_runtime_driver(runtime_driver);
