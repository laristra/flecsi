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

namespace flecsi {

/*----------------------------------------------------------------------------*
  Forward topology types.
 *----------------------------------------------------------------------------*/

namespace topology {

struct global_topology_t;
struct color_topology_t;

template<typename>
class mesh_topology_u;

} // namespace topology

namespace data {
namespace legion {

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_u<topology::global_topology_t> {

  using global_topology_t = topology::global_topology_t;
  using topology_reference_t = topology_reference_u<global_topology_t>;

  template<size_t NAMESPACE, size_t NAME>
  static topology_reference_t get_reference() {
    field_id_t fid;
    size_t identifier;
    topology_reference_t ref(fid, identifier);
    return ref;
  } // get_reference

}; // topology_u<topology::global_topology_t>

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_u<topology::color_topology_t> {

  using color_topology_t = topology::color_topology_t;
  using topology_reference_t = topology_reference_u<color_topology_t>;

  template<size_t NAMESPACE, size_t NAME>
  static topology_reference_t get_reference() {
    field_id_t fid;
    size_t identifier;
    topology_reference_t ref(fid, identifier);
    return ref;
  } // get_reference

}; // topology_u<topology::color_topology_t>

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
