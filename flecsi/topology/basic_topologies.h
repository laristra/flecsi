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

#include <flecsi/data/data_client.h>

namespace flecsi {
namespace topology {

/*!
  The global_topology_u type allows users to register data on a
  topology with a single index, i.e., there is one instance of
  the registered field type that is visible to all colors.

  @ingroup topology
 */

struct global_topology_u : public data::data_client_t {
  using type_identifier_t = global_topology_u;
}; // class global_topology_u

/*!
  The color_topology_u type allows users to register data on a
  per-color (similar to an MPI rank) basis. This topology will
  have one instance of each registered field type per color.

  @ingroup topology
 */

struct color_topology_u : public data::data_client_t {
  using type_identifier_t = color_topology_u;
}; // class color_topology_u

} // namespace topology
} // namespace flecsi
