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

#include "../client_handle.hh"
#include "../client_handle_specialization.hh"

namespace flecsi {
namespace data {
namespace mpi {

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<>
struct client_handle_specialization<topology::global_topology_t> {

  using client_t = topology::global_topology_t;

  template<size_t NAMESPACE, size_t NAME>
  static client_handle<client_t, 0> get_client_handle() {
    client_handle<client_t, 0> h;
    return h;
  } // get_client_handle

}; // client_handle_specialization<topology::global_topology_t>

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

template<>
struct client_handle_specialization<topology::index_topology_t> {

  using client_t = topology::index_topology_t;

  template<size_t NAMESPACE, size_t NAME>
  static client_handle<client_t, 0> get_client_handle() {
    client_handle<client_t, 0> h;
    return h;
  } // get_client_handle

}; // client_handle_specialization<topology::index_topology_t>

/*----------------------------------------------------------------------------*
  Mesh Topology.
 *----------------------------------------------------------------------------*/

#if 0
template<typename MESH_POLICY>
struct client_handle_specialization<topology::mesh_topology_t<MESH_POLICY>> {

  using client_t = topology::mesh_topology_t<MESH_POLICY>;

  template<size_t NAMESPACE, size_t NAME>
  static client_handle<client_t, 0> get_client_handle() {
    client_handle<client_t, 0> h;
    return h;
  } // get_client_handle

}; // client_handle_specialization<topology::mesh_topology_t<MESH_POLICY>>
#endif

} // namespace mpi
} // namespace data
} // namespace flecsi
