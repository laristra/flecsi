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

#include <flecsi/data/common/data_client.h>

namespace flecsi {
namespace topology {

/*!
  The internal_index_space_t enumeration sets reserved index space
  ids for the global and color topologies.
 */

// FIXME: user id generator
constexpr size_t global_index_space = 4096;
constexpr size_t color_index_space = 4097;

/*!
  The global_topology_u type allows users to register data on a
  topology with a single index, i.e., there is one instance of
  the registered field type that is visible to all colors.

  @ingroup topology
 */

struct global_topology_t : public data::data_client_t {
  using type_identifier_t = global_topology_t;
}; // struct global_topology_u

/*!
  The color_topology_u type allows users to register data on a
  per-color (similar to an MPI rank) basis. This topology will
  have one instance of each registered field type per color.

  @ingroup topology
 */

struct color_topology_t : public data::data_client_t {
  using type_identifier_t = color_topology_t;
}; // struct color_topology_u

} // namespace topology
} // namespace flecsi
