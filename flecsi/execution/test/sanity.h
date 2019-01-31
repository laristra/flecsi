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

#include <cinch-config.h>
#include <flecsi-config.h>

#if defined(FLECSI_ENABLE_FLOG)
#include <flecsi/utils/flog.h>
#endif

//#include <flecsi/execution/test/sanity_singleton.h>
#ifndef __FLECSI_PRIVATE__
  #define __FLECSI_PRIVATE__
#endif

#include <flecsi/execution/context.h>

#include <cinch/runtime.h>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

//sanity_register_id(0, 0);
//sanity_register_id(1, 1);
//sanity_register_id(2, 2);

#if defined(CINCH_ENABLE_BOOST)
inline void
flecsi_runtime_add_options(options_description & desc) {
} // add_options
#endif

#if defined(CINCH_ENABLE_BOOST)
inline int
flecsi_runtime_initialize(int argc, char ** argv, variables_map & vm) {
#else
inline int
flecsi_runtime_initialize(int argc, char ** argv) {
#endif

  MPI_Init(&argc, &argv);

  int rank{0};
  int size{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  flecsi::execution::context_t::instance().set_colors(size);

#if defined(FLECSI_ENABLE_FLOG)
  flog_init("all");
#endif
  flog(warn) << __FUNCTION__ << std::endl;
  return 0;
} // flecsi_runtime_initialize

inline int
flecsi_runtime_finalize(int argc, char ** argv,
  cinch::exit_mode_t mode) {
  cinch_function();

#if defined(FLECSI_ENABLE_FLOG)
  flog_finalize();
#endif

  MPI_Finalize();

  return 0;
} // runtime_finalize

inline cinch::runtime_handler_t runtime_handler {
  flecsi_runtime_initialize, flecsi_runtime_finalize
#if defined(CINCH_ENABLE_BOOST)
  ,
  flecsi_runtime_add_options
#endif
};

cinch_append_runtime_handler(runtime_handler);

inline int
flecsi_runtime_driver(int argc, char ** argv) {
  cinch_function();
  return 0;
} // flecsi_runtime_driver

cinch_register_runtime_driver(flecsi_runtime_driver);
