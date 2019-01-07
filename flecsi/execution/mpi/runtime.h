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

#include <cinch-config.h>
#include <flecsi-config.h>

#if defined(FLECSI_ENABLE_FLOG)
#include <flecsi/utils/flog.h>
#endif

#include <flecsi/execution/context.h>

#include <cinch/runtime.h>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <cstdlib>
#include <iostream>

#if defined(CINCH_ENABLE_BOOST)
#include <boost/program_options.hpp>
using namespace boost::program_options;
#endif

inline std::string __flecsi_tags = "all";

#if defined(CINCH_ENABLE_BOOST)
inline void
add_options(options_description & desc) {
#if defined(FLECSI_ENABLE_FLOG)
  desc.add_options()("tags,t", value(&__flecsi_tags)->implicit_value("0"),
    "Enable the specified output tags, e.g., --tags=tag1,tag2."
    " Passing --tags by itself will print the available tags.");
#endif
} // add_options
#endif

#if defined(CINCH_ENABLE_BOOST)
inline int
initialize(int argc, char ** argv, parsed_options & parsed) {
#else
inline int
initialize(int argc, char ** argv) {
#endif

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
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  flecsi::execution::context_t::instance().color() = rank;

#if defined(FLECSI_ENABLE_FLOG)
  flog_init(__flecsi_tags);
#endif

  return 0;
} // initialize

inline int
finalize(int argc, char ** argv, cinch::exit_mode_t mode) {

  MPI_Finalize();

  return 0;
} // initialize

inline cinch::runtime_handler_t handler {
  initialize, finalize
#if defined(CINCH_ENABLE_BOOST)
    ,
    add_options
#endif
};

cinch_append_runtime_handler(handler);

inline int
runtime_driver(int argc, char ** argv) {
  return flecsi::execution::context_t::instance().top_level_action()(
    argc, argv);
} // runtime_driver

cinch_register_runtime_driver(runtime_driver);
