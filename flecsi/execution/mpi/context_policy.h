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

namespace flecsi {
namespace execution {

struct mpi_context_policy_t {

  /*!
    Documnetation for this interface is in the top-level context type.
   */

  int start(int argc, char ** argv);

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

private:
  /*--------------------------------------------------------------------------*
    Runtime data members.
   *--------------------------------------------------------------------------*/

  size_t color_ = std::numeric_limits<size_t>::max();
  size_t colors_ = std::numeric_limits<size_t>::max();

}; // struct mpi_context_policy_t

} // namespace execution
} // namespace flecsi
