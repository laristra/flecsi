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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/run/types.hh"

#include <string>

namespace flecsi {
namespace topo {

struct canonical_base {
  enum index_space { vertices, cells };
  static constexpr std::size_t index_spaces = 2;

  struct coloring {

    struct local_coloring {};

    struct coloring_metadata {};

    local_coloring local_coloring_;
    coloring_metadata coloring_metadata_;

  }; // struct coloring

}; // struct canonical_base

} // namespace topo
} // namespace flecsi
