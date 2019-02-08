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

#define POLICY_NAMESPACE mpi
#include <flecsi/data/common/storage_class.h>
#undef POLICY_NAMESPACE

namespace flecsi {
namespace data {
namespace mpi {

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

#if 0
namespace unstructured_mesh_topology {

template<typename TYPE, size_t PRIVILEGES>
struct dense_handle_u {

  dense_handle_u() {}
  ~dense_handle_u() {}

}; // struct dense_handle_unstructured_mesh_u

} // namespace unstructured_mesh_topology

template<typename POLICY_TYPE>
struct storage_class_u<dense, flecsi::topology::mesh_topology_u<POLICY_TYPE>> {

  using client_t = flecsi::topology::mesh_topology_u<POLICY_TYPE>;

  template<typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static unstructured_mesh_topology::handle_t<DATA_TYPE, 0> get_handle(
    const client_handle_u<client_t, 0> & client_handle) {
  } // get_handle

}; // struct storage_class_u
#endif

} // namespace mpi
} // namespace data
} // namespace flecsi
