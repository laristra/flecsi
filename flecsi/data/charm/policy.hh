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
#include <flecsi/run/backend.hh>
#include <flecsi/run/types.hh>
#include <flecsi/topo/core.hh>
#include <flecsi/topo/unstructured/types.hh>
#include <flecsi/utils/flog.hh>

#if !defined(FLECSI_ENABLE_CHARM)
#error FLECSI_ENABLE_CHARM not defined! This file depends on Charm!
#endif

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

namespace charm {

#if 0
inline auto &
run() {
  return *Legion::Runtime::get_runtime();
}
inline auto
ctx() {
  return Legion::Runtime::get_context();
}

template<class T, void (Legion::Runtime::*D)(Legion::Context, T, bool)>
struct unique_handle {
  unique_handle() = default;
  unique_handle(T t) : h(t) {}
  unique_handle(unique_handle && u) noexcept : h(std::exchange(u.h, {})) {}
  ~unique_handle() {
    if(*this) // it's not clear whether empty handles can be deleted
      (run().*D)(ctx(), h, false);
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
using unique_index_space =
  unique_handle<Legion::IndexSpace, &Legion::Runtime::destroy_index_space>;
// Legion seems to be buggy with destroying partitions:
using unique_index_partition = Legion::IndexPartition;
using unique_field_space =
  unique_handle<Legion::FieldSpace, &Legion::Runtime::destroy_field_space>;
using unique_logical_region = unique_handle<Legion::LogicalRegion,
  &Legion::Runtime::destroy_logical_region>;
using unique_logical_partition = Legion::LogicalPartition;

inline unique_index_space
index1(std::size_t n) {
  return run().create_index_space(ctx(), Legion::Rect<1>(0, n - 1));
}
#endif

struct region {
  region(std::size_t n, const fields & fs) {}
#if 0
    : index_space(index1(n)),
      field_space([&fs] { // TIP: IIFE (q.v.) allows statements here
        auto & r = run();
        const auto c = ctx();
        unique_field_space ret = r.create_field_space(c);
        Legion::FieldAllocator allocator = r.create_field_allocator(c, ret);
        for(auto const & fi : fs)
          allocator.allocate_field(fi->type_size, fi->fid);
        return ret;
      }()),
      logical_region(
        run().create_logical_region(ctx(), index_space, field_space)) {}

  unique_index_space index_space;
  unique_field_space field_space;
  unique_logical_region logical_region;
#endif
};

struct partition {
  // TODO: support create_partition_by_image_range case
  template<class F>
  partition(const region & reg,
    std::size_t n,
    F f,
    disjointness dis = {},
    completeness cpt = {}) {}
#if 0
    : color_space(index1(n)),
      index_partition(run().create_partition_by_domain(
        ctx(),
        reg.index_space,
        [&] {
          std::map<Legion::DomainPoint, Legion::Domain> ret;
          for(std::size_t i = 0; i < n; ++i) {
            // NB: reg.index_space is assumed to be one-dimensional.
            const auto [b, e] = f(i);
            ret.try_emplace(i, Legion::Rect<1>(b, e - 1));
          }
          return ret;
        }(),
        color_space,
        true,
        Legion::PartitionKind((dis + 2) % 3 + (cpt + 3) % 3 * 3))),
      logical_partition(run().get_logical_partition(ctx(),
        reg.logical_region,
        index_partition)) {}
#endif

  std::size_t colors() const {
    //return run().get_index_space_domain(color_space).get_volume();
    return 1;
  }

#if 0
  unique_index_space color_space;
  unique_index_partition index_partition;
  unique_logical_partition logical_partition;
#endif

  template<topo::single_space>
  const partition & get_partition() const {
    return *this;
  }
};
} // namespace charm

using charm::region, charm::partition; // for backend-agnostic interface

} // namespace data
} // namespace flecsi
