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

namespace flecsi::runtime {

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

  int finalize();

  /*!
    Documnetation for this interface is in the top-level context type.
   */

  int start();

  /*!
    Documnetation for this interface is in the top-level context type.
   */

  size_t process() const {
    return color_;
  }

  size_t processes() const {
    return colors_;
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
    return color_;
  }

  /*
    Documnetation for this interface is in the top-level context type.
   */

  void set_color(size_t color) {
    color_ = color;
  }

  /*
    Documnetation for this interface is in the top-level context type.
   */

  size_t colors() const {
    return colors_;
  }

  /*
    Documnetation for this interface is in the top-level context type.
   */

  void set_colors(size_t colors) {
    colors_ = colors;
  }

  //--------------------------------------------------------------------------//
  // Reduction interface.
  //--------------------------------------------------------------------------//

  /*!
    Return the map of registered reduction types.
   */

  std::map<size_t, MPI_Datatype> & reduction_types() {
    return reduction_types_;
  } // reduction_types

private:
  /*--------------------------------------------------------------------------*
    Runtime data members.
   *--------------------------------------------------------------------------*/

  size_t color_ = std::numeric_limits<size_t>::max();
  size_t colors_ = std::numeric_limits<size_t>::max();

  /*--------------------------------------------------------------------------*
    Reduction data members.
   *--------------------------------------------------------------------------*/

  std::map<size_t, MPI_Datatype> reduction_types_;
};

} // namespace flecsi::runtime
