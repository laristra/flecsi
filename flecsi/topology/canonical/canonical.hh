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

#include <flecsi/runtime/types.hh>

#include <string>

namespace flecsi {
namespace topology {

struct canonical_base {

  struct coloring {

    struct local_coloring {};

    struct coloring_metadata {};

    coloring(std::string const & filename);

    local_coloring local_coloring_;
    coloring_metadata coloring_metadata_;

  }; // struct coloring

}; // struct canonical_base

/*!
  The canonical type is a dummy topology for development and testing.

  @ingroup topology
 */

template<typename TOPOLOGY_POLICY>
struct canonical : canonical_base {

  using coloring = canonical_base::coloring;

  canonical() = delete;

}; // struct canonical

} // namespace topology
} // namespace flecsi
