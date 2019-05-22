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
    auto & flecsi_context = execution::context_t::instance();

    auto & runtime_data =
      flecsi_context.index_topology_instance(topology_reference.identifier());

    runtime_data.colors = coloring.size();

    // Maybe this can go away
    runtime_data.index_space_id = unique_isid_t::instance().next();

    LegionRuntime::Arrays::Rect<1> bounds(0, coloring.size() - 1);
    Legion::Domain domain(Legion::Domain::from_rect<1>(bounds));

    Legion::IndexSpace index_space =
      legion_runtime->create_index_space(legion_context, domain);

    Legion::FieldSpace field_space = legion_runtime->create_field_space(legion_context);

    auto & field_info_store = flecsi_context.get_field_info_store(
      topology::index_topology_t::type_identifier_hash, index);

    Legion::FieldAllocator allocator = legion_runtime->create_field_allocator(legion_context, field_space);

    for(auto const & fi: field_info_store.data()) {
      allocator.allocate_field(fi.type_size, fi.fid);
    } // for

    runtime_data.logical_region = legion_runtime->create_logical_region(legion_context, index_space, field_space);

    LegionRuntime::Arrays::Blockify<1> color_block(1);
    Legion::IndexPartition index_partition = legion_runtime->create_index_partition(legion_context, index_space, color_block);
    runtime_data.color_partition = legion_runtime->get_logical_partition(legion_context, runtime_data.logical_region, index_partition);

  } // set_coloring

}; // struct topology_instance_u<topology::index_topology_t>

/*----------------------------------------------------------------------------*
  Mesh Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_instance_u<topology::mesh_topology_u<POLICY_TYPE>> {

  using topology_reference_t =
    topology_reference_u<topology::mesh_topology_u<POLICY_TYPE>>;

}; // struct topology_instance_u<topology::mesh_topology_u<POLICY_TYPE>>

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
