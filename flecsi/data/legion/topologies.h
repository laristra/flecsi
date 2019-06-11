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
#include <flecsi/utils/flog.h>

#define POLICY_NAMESPACE legion
#include <flecsi/data/common/topology.h>
#undef POLICY_NAMESPACE

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(topologies);

namespace flecsi {

/*----------------------------------------------------------------------------*
  Forward topology types.
 *----------------------------------------------------------------------------*/

namespace topology {

struct global_topology_t;
struct index_topology_t;

template<typename>
class ntree_topology_u;

template<typename>
class set_topology_u;

template<typename>
class structured_mesh_topology_u;

template<typename>
class unstructured_mesh_topology_u;

} // namespace topology

namespace data {
namespace legion {

using namespace topology;

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_instance_u<index_topology_t> {

  using topology_reference_t = topology_reference_u<index_topology_t>;

  static void set_coloring(topology_reference_t const & topology_reference,
    index_topology_t::coloring_t const & coloring) {

    {
      flog_tag_guard(topologies);
      flog(internal) << "Set coloring for " << topology_reference.identifier()
                     << std::endl;
    }

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

    Legion::FieldSpace field_space =
      legion_runtime->create_field_space(legion_context);

    auto & field_info_store = flecsi_context.get_field_info_store(
      index_topology_t::type_identifier_hash, storage_label_t::index);

    Legion::FieldAllocator allocator =
      legion_runtime->create_field_allocator(legion_context, field_space);

    for(auto const & fi : field_info_store.field_info()) {
      allocator.allocate_field(fi.type_size, fi.fid);
    } // for

    runtime_data.logical_region = legion_runtime->create_logical_region(
      legion_context, index_space, field_space);

    Legion::IndexPartition index_partition =
      legion_runtime->create_equal_partition(
        legion_context, index_space, index_space);

    runtime_data.color_partition = legion_runtime->get_logical_partition(
      legion_context, runtime_data.logical_region, index_partition);

  } // set_coloring

}; // index_topology_t specialization

/*----------------------------------------------------------------------------*
  NTree Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_instance_u<ntree_topology_u<POLICY_TYPE>> {

  using topology_reference_t =
    topology_reference_u<ntree_topology_u<POLICY_TYPE>>;
  using coloring_t = typename ntree_topology_u<POLICY_TYPE>::coloring_t;

  static void set_coloring(topology_reference_t const & topology_reference,
    coloring_t const & coloring) {} // set_coloring

}; // ntree_topology_u specialization

/*----------------------------------------------------------------------------*
  Set Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_instance_u<set_topology_u<POLICY_TYPE>> {

  using topology_reference_t =
    topology_reference_u<set_topology_u<POLICY_TYPE>>;

}; // set_topology_u specialization

/*----------------------------------------------------------------------------*
  Structured Mesh Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_instance_u<structured_mesh_topology_u<POLICY_TYPE>> {

  using topology_reference_t =
    topology_reference_u<structured_mesh_topology_u<POLICY_TYPE>>;

}; // structured_mesh_topology_u specialization

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_instance_u<unstructured_mesh_topology_u<POLICY_TYPE>> {

  using topology_reference_t =
    topology_reference_u<unstructured_mesh_topology_u<POLICY_TYPE>>;

}; // unstructured_mesh_topology_u specialization

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
