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
#endif

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include "../context.hh"

#include <boost/program_options.hpp>
#include <mpi.h>

#include <map>

namespace flecsi::run {

struct context_t : context {

  //--------------------------------------------------------------------------//
  //  Runtime.
  //--------------------------------------------------------------------------//

  /*!
    Documnetation for this interface is in the top-level context type.
   */

  int initialize(int argc, char ** argv, bool dependent);

  /*!
    Documnetation for this interface is in the top-level context type.
   */

  void finalize();

  /*!
    Documnetation for this interface is in the top-level context type.
   */

  int start(const std::function<int()> &);

  /*!
    Documnetation for this interface is in the top-level context type.
   */

  size_t process() const {
    return process_;
  }

  size_t processes() const {
    return processes_;
  }

  size_t threads_per_process() const {
    return 1;
  }

  size_t threads() const {
    return 0;
  }

  /*
    Documnetation for this interface is in the top-level context type.
   */

  static size_t task_depth() {
    return 0;
  } // task_depth

  /*
    Documnetation for this interface is in the top-level context type.
   */

  size_t color() const {
    return process_;
  }

  /*
    Documnetation for this interface is in the top-level context type.
   */

  size_t colors() const {
    return processes_;
  }
};

} // namespace flecsi::run
