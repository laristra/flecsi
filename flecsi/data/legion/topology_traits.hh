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

#include <flecsi/data/data_reference.hh>
#include <flecsi/data/storage_classes.hh>
#include <flecsi/runtime/backend.hh>
#include <flecsi/topology/core.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/topology/unstructured_mesh/types.hh>
#include <flecsi/utils/flog.hh>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(topologies);

namespace flecsi {
namespace data {
// FIXME: get rid of this namespace
namespace legion {

template<typename TOPOLOGY_TYPE>
struct topology_traits;

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_traits<topology::index_topology_t> {

  static void create(data_reference_base_t const & topology_reference,
    data_reference_base_t const & coloring_reference) {
    {
      flog_tag_guard(topologies);
      flog_devel(info) << "Set coloring for " << topology_reference.identifier()
                       << std::endl;
    }

    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();
    auto & flecsi_context = runtime::context_t::instance();

    auto & runtime_data =
      flecsi_context.index_topology_instance(topology_reference.identifier());

    auto const & coloring =
      flecsi_context.index_coloring(coloring_reference.identifier());

    runtime_data.colors = coloring.size();

    // Maybe this line can go away
    runtime_data.index_space_id = unique_isid_t::instance().next();

    LegionRuntime::Arrays::Rect<1> bounds(0, coloring.size() - 1);
    Legion::Domain domain(Legion::Domain::from_rect<1>(bounds));

    runtime_data.index_space =
      legion_runtime->create_index_space(legion_context, domain);

    runtime_data.field_space =
      legion_runtime->create_field_space(legion_context);

    auto & field_info_store = flecsi_context.get_field_info_store(
      topology::id<topology::index_topology_t>(), storage_label_t::dense);

    Legion::FieldAllocator allocator = legion_runtime->create_field_allocator(
      legion_context, runtime_data.field_space);

    for(auto const & fi : field_info_store.field_info()) {
      allocator.allocate_field(fi.type_size, fi.fid);
    } // for

    runtime_data.logical_region = legion_runtime->create_logical_region(
      legion_context, runtime_data.index_space, runtime_data.field_space);

    Legion::IndexPartition index_partition =
      legion_runtime->create_equal_partition(
        legion_context, runtime_data.index_space, runtime_data.index_space);

    runtime_data.color_partition = legion_runtime->get_logical_partition(
      legion_context, runtime_data.logical_region, index_partition);
  } // create

  static void destroy(data_reference_base_t const & topology_reference) {

    {
      flog_tag_guard(topologies);
      flog_devel(info) << "Set coloring for " << topology_reference.identifier()
                       << std::endl;
    }

    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();
    auto & flecsi_context = runtime::context_t::instance();

    auto & runtime_data =
      flecsi_context.index_topology_instance(topology_reference.identifier());

    legion_runtime->destroy_logical_region(
      legion_context, runtime_data.logical_region);

    legion_runtime->destroy_field_space(
      legion_context, runtime_data.field_space);

    legion_runtime->destroy_index_space(
      legion_context, runtime_data.index_space);

  } // destroy

}; // index_topology_t specialization

/*----------------------------------------------------------------------------*
  NTree Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_traits<topology::ntree_topology<POLICY_TYPE>> {

  // Distribute the entities on the different processes
  // Create the tree data structure locally
  static void create(data_reference_base_t const & topology_reference,
    data_reference_base_t const & coloring_reference) {

    {
      flog_tag_guard(topologies);
      flog_devel(info) << "Set coloring for " << topology_reference.identifier()
                       << std::endl;
    }

  } // create

  // Update the entities position in the tree
  static void update(data_reference_base_t const & topology_reference,
    data_reference_base_t const & coloring_reference) {

    {
      flog_tag_guard(topologies);
      flog_devel(info) << "Update coloring for "
                       << topology_reference.identifier() << std::endl;
    }

  } // update

  static void destroy(data_reference_base_t const & topology_reference) {

    {
      flog_tag_guard(topologies);
      flog_devel(info) << "Destroy coloring for "
                       << topology_reference.identifier() << std::endl;
    }

  } // destroy

}; // ntree_topology specialization

/*----------------------------------------------------------------------------*
  Set Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_traits<topology::set_topology<POLICY_TYPE>> {

}; // set_topology specialization

/*----------------------------------------------------------------------------*
  Structured Mesh Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_traits<topology::structured_mesh_topology<POLICY_TYPE>> {

}; // structured_mesh_topology specialization

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

template<typename POLICY_TYPE>
struct topology_traits<topology::unstructured_mesh_topology<POLICY_TYPE>> {

  using topology_t = topology::unstructured_mesh_topology<POLICY_TYPE>;

#if 0
  struct entity_walker_t : public utils::tuple_walker<index_walker_t> {

    entity_walker_t(coloring_t const & coloring) : coloring_(coloring) {}

    template<typename ENTITY_TYPE>
    void visit_type() {
    } // visit_type

  private:
    coloring_t coloring_;

  }; // struct entity_walker_t
#endif

  static void create(data_reference_base_t const & topology_reference,
    data_reference_base_t const & coloring_reference) {

    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();
    auto & flecsi_context = runtime::context_t::instance();

    auto & dense_field_info_store = flecsi_context.get_field_info_store(
      topology::id<topology_t>(), storage_label_t::dense);

#if 0
    for(size_t is{0}; is<coloring.index_spaces; ++is) {

      for(auto const & fi : field_info_store.field_info()) {
        allocator.allocate_field(fi.type_size, fi.fid);
      } // for


    } // for

    auto & ragged_field_info_store = flecsi_context.get_field_info_store(
      /* topology_t */, storage_label_t::ragged);

    auto & sparse_field_info_store = flecsi_context.get_field_info_store(
      /* topology_t */, storage_label_t::sparse);

#endif
  } // create

  static void destroy(data_reference_base_t const & topology_reference) {}

}; // unstructured_mesh_topology specialization

// NOTE THAT THE HANDLE TYPE FOR THIS TYPE WILL NEED TO CAPTURE THE
// UNDERLYING TOPOLOGY TYPE, i.e., topology::mesh_topology_t<MESH_POLICY>

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

} // namespace legion
} // namespace data
} // namespace flecsi
