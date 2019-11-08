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

#include <flecsi/data/legion/types.hh>
#include <flecsi/data/reference.hh>
#include <flecsi/data/storage_classes.hh>
#include <flecsi/runtime/backend.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/topology/core.hh>
#include <flecsi/topology/unstructured_mesh/types.hh>
#include <flecsi/utils/flog.hh>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(topologies);

namespace flecsi {
namespace data {

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

inline topology_data<topology::index_t>::topology_data(
  const type::coloring & coloring)
  : colors(coloring.size()) {

  auto legion_runtime = Legion::Runtime::get_runtime();
  auto legion_context = Legion::Runtime::get_context();
  auto & flecsi_context = runtime::context_t::instance();

  LegionRuntime::Arrays::Rect<1> bounds(0, coloring.size() - 1);
  Legion::Domain domain(Legion::Domain::from_rect<1>(bounds));

  index_space = legion_runtime->create_index_space(legion_context, domain);

  field_space = legion_runtime->create_field_space(legion_context);

  auto & field_info_store = flecsi_context.get_field_info_store(
    topology::id<topology::index_t>(), storage_label_t::dense);

  Legion::FieldAllocator allocator =
    legion_runtime->create_field_allocator(legion_context, field_space);

  for(auto const & fi : field_info_store) {
    allocator.allocate_field(fi.type_size, fi.fid);
  } // for

  logical_region = legion_runtime->create_logical_region(
    legion_context, index_space, field_space);

  Legion::IndexPartition index_partition =
    legion_runtime->create_equal_partition(
      legion_context, index_space, index_space);

  color_partition = legion_runtime->get_logical_partition(
    legion_context, logical_region, index_partition);
}

inline legion::topology_base::~topology_base() {
  auto legion_runtime = Legion::Runtime::get_runtime();
  auto legion_context = Legion::Runtime::get_context();

  legion_runtime->destroy_logical_region(legion_context, logical_region);

  legion_runtime->destroy_field_space(legion_context, field_space);

  legion_runtime->destroy_index_space(legion_context, index_space);

} // deallocate

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

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

inline topology_data<topology::unstructured_mesh_base_t>::topology_data(
  const type::coloring & coloring) {

  auto legion_runtime = Legion::Runtime::get_runtime();
  auto legion_context = Legion::Runtime::get_context();
  auto & flecsi_context = runtime::context_t::instance();

  auto & dense_field_info_store = flecsi_context.get_field_info_store(
    topology::id<type>(), storage_label_t::dense);

#if 0
    for(size_t is{0}; is<coloring.index_spaces; ++is) {

      for(auto const & fi : field_info_store) {
        allocator.allocate_field(fi.type_size, fi.fid);
      } // for


    } // for

    auto & ragged_field_info_store = flecsi_context.get_field_info_store(
      /* type */, storage_label_t::ragged);

    auto & sparse_field_info_store = flecsi_context.get_field_info_store(
      /* type */, storage_label_t::sparse);

#endif
}

// NOTE THAT THE HANDLE TYPE FOR THIS TYPE WILL NEED TO CAPTURE THE
// UNDERLYING TOPOLOGY TYPE, i.e., topology::mesh_t<MESH_POLICY>

#if 0
template<typename MESH_POLICY>
struct client_handle_specialization<topology::mesh_t<MESH_POLICY>> {

  using client_t = topology::mesh_t<MESH_POLICY>;

  template<size_t NAMESPACE, size_t NAME>
  static client_handle<client_t, 0> get_client_handle() {
    client_handle<client_t, 0> h;
    return h;
  } // get_client_handle

}; // client_handle_specialization<topology::mesh_t<MESH_POLICY>>
#endif

} // namespace data
} // namespace flecsi
