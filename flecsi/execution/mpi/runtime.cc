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
/*! @file */

#include <flecsi-config.h>

#define __FLECSI_PRIVATE__
#if defined(FLECSI_ENABLE_FLOG)
#include <flecsi/utils/flog.hh>
#endif

#include "../command_line_options.hh"
#include "flecsi/runtime.hh"
#include "flecsi/runtime/backend.hh"

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <cstdlib>
#include <iostream>

#include <boost/program_options.hpp>
using namespace boost::program_options;

using namespace flecsi;

void
flecsi_mpi_add_options(options_description & desc) {
  options_description flecsi("FleCSI Runtime Options");
  flecsi.add_options()
#if defined(FLECSI_ENABLE_FLOG)
    FLECSI_FLOG_TAG_OPTION
#endif
    ;

  desc.add(flecsi);
} // add_options

int
flecsi_mpi_initialize(int argc, char ** argv, variables_map & vm) {

#if defined(FLECSI_ENABLE_FLOG)
  if(__flog_tags == "0") {
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
  flecsi::runtime::context_t::instance().set_color(rank);
  flecsi::runtime::context_t::instance().set_colors(size);

#if defined(FLECSI_ENABLE_FLOG)
  flog_initialize(__flog_tags);
#endif

  return 0;
} // initialize

int
flecsi_mpi_finalize(int argc, char ** argv, exit_mode_t mode) {

#if defined(FLECSI_ENABLE_FLOG)
  flog_finalize();
#endif

  MPI_Finalize();

  return 0;
} // initialize

runtime_handler_t flecsi_mpi_handler{flecsi_mpi_initialize,
  flecsi_mpi_finalize,
  flecsi_mpi_add_options};

flecsi_append_runtime_handler(flecsi_mpi_handler);

int
flecsi_mpi_runtime_driver(int argc, char ** argv, variables_map & vm) {
  return flecsi::runtime::context_t::instance().start(argc, argv, vm);
} // runtime_driver

flecsi_register_runtime_driver(flecsi_mpi_runtime_driver);
