/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#if defined(FLECSI_ENABLE_FLOG)
#include <flecsi/utils/flog.h>
#endif

#include <flecsi/execution/common/command_line_options.h>
#include <flecsi/execution/context.h>
#endif

#include <cinch/runtime.h>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <cstdlib>
#include <iostream>

#include <boost/program_options.hpp>
using namespace boost::program_options;

inline void
flecsi_legion_add_options(options_description & desc) {
  options_description flecsi("FleCSI Runtime Options");
  flecsi.add_options()
#if defined(FLECSI_ENABLE_FLOG)
    FLECSI_FLOG_TAG_OPTION
#endif
    FLECSI_THREADS_PER_PROCESS_OPTION;

  desc.add(flecsi);
} // add_options

inline int
flecsi_legion_initialize(int argc, char ** argv, variables_map & vm) {

#if defined(FLECSI_ENABLE_FLOG)
  if(__flecsi_tags == "0") {
    std::cout << "Available tags (FLOG):" << std::endl;

    for(auto t : flog_tag_map()) {
      std::cout << " " << t.first << std::endl;
    } // for

    return 1;
  } // if
#endif

  int version, subversion;
  MPI_Get_version(&version, &subversion);

#if defined(GASNET_CONDUIT_MPI)
  if(version == 3 && subversion > 0) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    if(provided < MPI_THREAD_MULTIPLE) {
      std::cerr << "Your implementation of MPI does not support "
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

#if defined(FLECSI_ENABLE_FLOG)
  flog_initialize(__flecsi_tags);
#endif

  return 0;
} // initialize

inline int
flecsi_legion_finalize(int argc, char ** argv, cinch::exit_mode_t mode) {

#if defined(FLECSI_ENABLE_FLOG)
  flog_finalize();
#endif

// Shutdown the MPI runtime
#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif

  return 0;
} // initialize

inline cinch::runtime_handler_t flecsi_legion_handler{flecsi_legion_initialize,
  flecsi_legion_finalize,
  flecsi_legion_add_options};

cinch_append_runtime_handler(flecsi_legion_handler);

inline int
flecsi_legion_runtime_driver(int argc, char ** argv, variables_map & vm) {
  return flecsi::execution::context_t::instance().start(argc, argv, vm);
} // runtime_driver

cinch_register_runtime_driver(flecsi_legion_runtime_driver);
