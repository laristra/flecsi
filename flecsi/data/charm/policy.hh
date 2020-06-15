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

#include <flecsi/data/charm/types.hh>
#include <flecsi/data/reference.hh>
#include <flecsi/data/storage_classes.hh>
#include <flecsi/runtime/backend.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/topology/core.hh>
#include <flecsi/topology/unstructured/types.hh>
#include <flecsi/utils/flog.hh>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(topologies);

namespace flecsi {
namespace data {

template<class C>
struct topology_id {
  // NB: C-style cast supports private inheritance
  topology_id() : id(runtime::context_t::instance().record(*(C *)this)) {}
  topology_id(const topology_id &) : topology_id() {}
  ~topology_id() {
    runtime::context_t::instance().forget(id);
  }
  topology_id & operator=(const topology_id &) noexcept {
    return *this;
  }

  std::size_t id;
};

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

inline topology_data<topology::index>::topology_data(
  const type::coloring & coloring)
  : topology_base(Legion::Domain::from_rect<1>(
      LegionRuntime::Arrays::Rect<1>(0, coloring.size() - 1))),
    colors(coloring.size()) {

  auto legion_runtime = Legion::Runtime::get_runtime();
  auto legion_context = Legion::Runtime::get_context();
  auto & flecsi_context = runtime::context_t::instance();

  auto & field_info_store = flecsi_context.get_field_info_store(
    topology::id<topology::index>(), storage_label_t::dense);

  Legion::FieldAllocator allocator =
    legion_runtime->create_field_allocator(legion_context, field_space);

  for(auto const & fi : field_info_store) {
    allocator.allocate_field(fi->type_size, fi->fid);
  } // for

  allocate();

  Legion::IndexPartition index_partition =
    legion_runtime->create_equal_partition(
      legion_context, index_space, index_space);

  color_partition = legion_runtime->get_logical_partition(
    legion_context, logical_region, index_partition);
}

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

inline topology_data<topology::unstructured_base>::topology_data(
  const type::coloring & coloring) {
  (void)coloring;
}

// NOTE THAT THE HANDLE TYPE FOR THIS TYPE WILL NEED TO CAPTURE THE
// UNDERLYING TOPOLOGY TYPE, i.e., topology::mesh_t<MESH_POLICY>

} // namespace data
} // namespace flecsi
