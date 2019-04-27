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

namespace flecsi {
namespace topology {

// FIXME: user id generator
constexpr size_t color_index_space = 4097;

/*!
  The color_topology_u type allows users to register data on a
  per-color (similar to an MPI rank) basis. This topology will
  have one instance of each registered field type per color.

  @ingroup topology
 */

struct color_topology_t {
  using type_identifier_t = color_topology_t;
}; // struct color_topology_u

} // namespace topology
} // namespace flecsi
