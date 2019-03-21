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

#define POLICY_NAMESPACE legion
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

namespace global_topology {

struct topology_handle_t {

  using topology_type_t = topology::global_topology_t;

}; // struct topology_handle_t

template<size_t PRIVILEGES>
struct topology_accessor_t {
  topology_accessor_t(topology_handle_t & handle)
    : handle_(handle) {}

private:

  topology_handle_t & handle_;

}; // struct topology_accessor_t

} // namespace global_topology

template<>
struct topology_u<topology::global_topology_t> {

  using topology_handle_t = global_topology::topology_handle_t;

  template<size_t NAMESPACE, size_t NAME>
  static topology_handle_t get_topology_handle() {
    topology_handle_t h;
    return h;
  } // get_topology_handle

}; // topology_u<topology::global_topology_t>

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

namespace color_topology {

struct topology_handle_t {
  using topology_type_t = topology::color_topology_t;
}; // struct topology_handle_t

} // namespace color_topology

template<>
struct topology_u<topology::color_topology_t> {

  using topology_handle_t = color_topology::topology_handle_t;

  template<size_t NAMESPACE, size_t NAME>
  static topology_handle_t get_topology_handle() {
    topology_handle_t h;
    return h;
  } // get_topology_handle

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
