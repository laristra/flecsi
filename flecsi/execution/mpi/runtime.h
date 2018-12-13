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
#include <flecsi/execution/context.h>

#include <cinch/runtime.h>

#if !defined(FLECSI_ENABLE_MPI)
  #error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <cstdlib>
#include <iostream>

/*
  Initialization for MPI backend
 */

inline int initialize(int argc, char ** argv) {
  std::cout << "Executing initialize" << std::endl;

  // Initialize MPI runtime
  MPI_Init(&argc, &argv);

  // Get the rank
  int rank{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  flecsi::execution::context_t::instance().color() = rank;

  return 0;
} // initialize

/*
  Finalization for MPI backend
 */

inline int finalize(int argc, char ** argv, cinch::exit_mode_t mode) {
  std::cout << "Executing finalize with mode " << size_t{mode} << std::endl;

  // Shutdown MPI runtime
  MPI_Finalize();

  return 0;
} // initialize

inline bool output(int argc, char ** argv) {
  return flecsi::execution::context_t::instance().color() == 0;
} // output

inline cinch::runtime_handler_t handler{
  initialize,
  finalize,
  output
};

cinch_append_runtime_handler(handler);

inline int runtime_driver(int argc, char ** argv) {
  return 0;
} // runtime_driver

cinch_register_runtime_driver(runtime_driver);
