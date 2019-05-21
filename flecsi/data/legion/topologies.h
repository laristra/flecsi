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

#include <flecsi/data/common/data_reference.h>
#include <flecsi/execution/context.h>
#include <flecsi/runtime/types.h>

#define POLICY_NAMESPACE legion
#include <flecsi/data/common/topology.h>
#undef POLICY_NAMESPACE

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

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

  using topology_reference_t = topology_reference_u<topology::index_topology_t>;

  static void set_coloring(topology_reference_t const & topology_reference,
    topology::index_topology_t::coloring_t const & coloring) {

    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();

    auto & data_ = execution::context_t::instance().index_topology_instance(
      topology_reference.identifier());

    data_.index_space_id = unique_isid_t::instance().next();

  } // set_coloring

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
