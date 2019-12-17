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

// #if !defined(FLECSI_ENABLE_HPX)
// #error FLECSI_ENABLE_HPX not defined! This file depends on HPX!
// #endif

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include <mpi.h>

#include <vector>
#include <string>
#include <stdexcept>

#include "../context.hh"

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
    return context::process_;
  }

  size_t processes() const {
    return context::processes_;
  }

  size_t threads_per_process() const {
    return context::threads_per_process_;
  }

  size_t threads() const {
    return context::threads_;
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


  // TODO: Set the value for color, colors, process and processes
  // according to what Rod tells

  // Currently it returns the process id.
  size_t color() const {
    return hpx::get_locality_id();
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
    return hpx::find_all_localities().size();
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


private:
  /*--------------------------------------------------------------------------*
    Runtime data members.
   *--------------------------------------------------------------------------*/

  // Currently working according to MPI color scheme
  size_t color_ = std::numeric_limits<size_t>::max();
  size_t colors_ = std::numeric_limits<size_t>::max();

};

} // namespace flecsi::runtime
