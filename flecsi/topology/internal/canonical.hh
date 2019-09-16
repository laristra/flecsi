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

namespace flecsi {
namespace topology {

struct canonical_topology_base_t {

  struct coloring_t {

    struct local_coloring_t {};

    struct coloring_metadata_t {};

    static void color(local_coloring_t & local_coloring,
      coloring_metadata_t & coloring_metadata) {} // color

    local_coloring_t local_coloring_;
    coloring_metadata_t coloring_metadata_;

  }; // struct coloring_t

}; // struct canonical_topology_base_t

/*!
  The canonical_topology type is a dummy topology for development and testing.

  @ingroup topology
 */

template<typename TOPOLOGY_POLICY>
struct canonical_topology {

  canonical_topology() = delete;

}; // struct canonical_topology

} // namespace topology
} // namespace flecsi
