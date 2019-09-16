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

struct canonical_topology_base {

  struct coloring {

    struct local_coloring {};

    struct coloring_metadata {};

    static void color(coloring & coloring_info, std::string const & filename);

    local_coloring local_coloring_;
    coloring_metadata coloring_metadata_;

  }; // struct coloring

}; // struct canonical_topology_base

/*!
  The canonical_topology type is a dummy topology for development and testing.

  @ingroup topology
 */

template<typename TOPOLOGY_POLICY>
struct canonical_topology : public canonical_topology_base {

  using coloring = canonical_topology_base::coloring;

  canonical_topology() = delete;

}; // struct canonical_topology

} // namespace topology
} // namespace flecsi
