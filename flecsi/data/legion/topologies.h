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

#define POLICY_NAMESPACE legion
#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/common/topology.h>
#undef POLICY_NAMESPACE

#include <legion.h>

namespace flecsi {

/*----------------------------------------------------------------------------*
  Forward topology types.
 *----------------------------------------------------------------------------*/

namespace topology {

struct global_topology_t;
struct index_topology_t;

template<typename>
class mesh_topology_u;

} // namespace topology

namespace data {
namespace legion {

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_instance_u<topology::index_topology_t> {

}; // topology_instance_u<topology::index_topology_t>

/*----------------------------------------------------------------------------*
  Mesh Topology.
 *----------------------------------------------------------------------------*/

// NOTE THAT THE HANDLE TYPE FOR THIS TYPE WILL NEED TO CAPTURE THE
// UNDERLYING TOPOLOGY TYPE, i.e., topology::mesh_topology_t<MESH_POLICY>

#if 0
template<typename MESH_POLICY>
struct client_handle_specialization_u<topology::mesh_topology_t<MESH_POLICY>> {

  using client_t = topology::mesh_topology_t<MESH_POLICY>;

  template<size_t NAMESPACE, size_t NAME>
  static client_handle_u<client_t, 0> get_client_handle() {
    client_handle_u<client_t, 0> h;
    return h;
  } // get_client_handle

}; // client_handle_specialization_u<topology::mesh_topology_t<MESH_POLICY>>
#endif

} // namespace legion
} // namespace data
} // namespace flecsi
