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

#define POLICY_NAMESPACE mpi
#include <flecsi/data/common/storage_class.hh>
#undef POLICY_NAMESPACE

namespace flecsi {
namespace data {
namespace mpi {

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

#if 0
namespace unstructured_mesh_topo {

template<typename TYPE, size_t PRIVILEGES>
struct dense_handle {

  dense_handle() {}
  ~dense_handle() {}

}; // struct dense_handle_unstructured_mesh

} // namespace unstructured_mesh_topo

template<typename POLICY_TYPE>
struct storage_class<dense, flecsi::topology::mesh_topology<POLICY_TYPE>> {

  using client_t = flecsi::topology::mesh_topology<POLICY_TYPE>;

  template<typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static unstructured_mesh_topo::handle_t<DATA_TYPE, 0> get_handle(
    const client_handle<client_t, 0> & client_handle) {
  } // get_handle

}; // struct storage_class
#endif

} // namespace mpi
} // namespace data
} // namespace flecsi
