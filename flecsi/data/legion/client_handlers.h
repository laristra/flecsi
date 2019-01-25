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

#include <flecsi/data/common/client_handler.h>
#include <flecsi/data/legion/client_handle.h>

namespace flecsi {

/*----------------------------------------------------------------------------*
  Forward topology types.
 *----------------------------------------------------------------------------*/

namespace topology {
struct global_topology_u;
struct color_topology_u;
template<typename>
class mesh_topology_u;
} // namespace topology

namespace data {

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<>
struct client_handler_u<topology::global_topology_u> {

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static client_handle_u<DATA_CLIENT_TYPE, 0> get_client_handle() {
    return 0;
  } // get_client_handle

}; // client_handler_u<topology::global_topology_u>

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

template<>
struct client_handler_u<topology::color_topology_u> {

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static client_handle_u<DATA_CLIENT_TYPE, 0> get_client_handle() {
    return 0;
  } // get_client_handle

}; // client_handler_u<topology::color_topology_u>

/*----------------------------------------------------------------------------*
  Mesh Topology.
 *----------------------------------------------------------------------------*/

template<typename MESH_POLICY>
struct client_handler_u<topology::mesh_topology_u<MESH_POLICY>> {

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static client_handle_u<DATA_CLIENT_TYPE, 0> get_client_handle() {
    return 0;
  } // get_client_handle

}; // client_handler_u<topology::mesh_topology_u<MESH_POLICY>>

} // namespace data
} // namespace flecsi