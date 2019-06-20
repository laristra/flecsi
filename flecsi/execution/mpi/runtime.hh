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

#include <cinch-config.h>
#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#if defined(FLECSI_ENABLE_FLOG)
#include <flecsi/utils/flog.hh>
#endif

#include <flecsi/execution/common/command_line_options.hh>
#include <flecsi/execution/context.hh>
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
flecsi_mpi_add_options(options_description & desc) {
  options_description flecsi("FleCSI Runtime Options");
  flecsi.add_options()
#if defined(FLECSI_ENABLE_FLOG)
    FLECSI_FLOG_TAG_OPTION
#endif
    ;

  desc.add(flecsi);
} // add_options

inline int
flecsi_mpi_initialize(int argc, char ** argv, variables_map & vm) {

#if defined(FLECSI_ENABLE_FLOG)
  if(__flecsi_tags == "0") {
    std::cout << "Available tags (FLOG):" << std::endl;

    for(auto t : flog_tag_map()) {
      std::cout << " " << t.first << std::endl;
    } // for

    return 1;
  } // if
#endif

  MPI_Init(&argc, &argv);

  int rank{0};
  int size{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  flecsi::execution::context_t::instance().set_color(rank);
  flecsi::execution::context_t::instance().set_colors(size);

#if defined(FLECSI_ENABLE_FLOG)
  flog_initialize(__flecsi_tags);
#endif

  return 0;
} // initialize

inline int
flecsi_mpi_finalize(int argc, char ** argv, cinch::exit_mode_t mode) {

#if defined(FLECSI_ENABLE_FLOG)
  flog_finalize();
#endif

  MPI_Finalize();

  return 0;
} // initialize

inline cinch::runtime_handler_t flecsi_mpi_handler{flecsi_mpi_initialize,
  flecsi_mpi_finalize,
  flecsi_mpi_add_options};

cinch_append_runtime_handler(flecsi_mpi_handler);

inline int
flecsi_mpi_runtime_driver(int argc, char ** argv) {
  return flecsi::execution::context_t::instance().start(argc, argv);
} // runtime_driver

cinch_register_runtime_driver(flecsi_mpi_runtime_driver);
