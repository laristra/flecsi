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

/*!  @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <flecsi/runtime/backend.hh>
#include <flecsi/topology/base.hh>

#include <legion.h>

#include <unordered_map>

namespace flecsi {
namespace data {

namespace legion {
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
  unique_handle & operator=(unique_handle u) noexcept {
    std::swap(h, u.h);
    return *this;
  }
  explicit operator bool() {
    return h.exists();
  }
  operator T() const {
    return h;
  }

private:
  T h;
};

using unique_index_space =
  unique_handle<Legion::IndexSpace, &Legion::Runtime::destroy_index_space>;
using unique_field_space =
  unique_handle<Legion::FieldSpace, &Legion::Runtime::destroy_field_space>;
using unique_logical_region = unique_handle<Legion::LogicalRegion,
  &Legion::Runtime::destroy_logical_region>;
// TODO: do we need to destroy_logical_partition?

struct topology_base {
  topology_base(Legion::Domain d)
    : index_space(run().create_index_space(ctx(), d)) {}
  size_t index_space_id = unique_isid_t::instance().next(); // TODO: needed?
  unique_index_space index_space;
  unique_field_space field_space = run().create_field_space(ctx());
  unique_logical_region logical_region;

protected:
  void allocate() {
    logical_region =
      run().create_logical_region(ctx(), index_space, field_space);
  }
};
} // namespace legion

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::global_t> : legion::topology_base {
  using type = topology::global_t;
  topology_data(const type::coloring &)
    : topology_base(Legion::Domain::from_rect<1>(
        LegionRuntime::Arrays::Rect<1>(LegionRuntime::Arrays::Point<1>(0),
          LegionRuntime::Arrays::Point<1>(1)))) {
    Legion::FieldAllocator allocator =
      legion::run().create_field_allocator(legion::ctx(), field_space);

    /*
      Note: This call to get_field_info_store uses the non-const version
      so that this call works if no fields have been registered. In other parts
      of the code that occur after initialization, the const version of this
      call should be used.
     */

    auto & field_info_store =
      runtime::context_t::instance().get_field_info_store(
        topology::id<topology::global_t>(), storage_label_t::dense);

    for(auto const & fi : field_info_store) {
      allocator.allocate_field(fi->type_size, fi->fid);
    } // for

    allocate();
  }
};

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::index_t> : legion::topology_base {
  using type = topology::index_t;
  topology_data(const type::coloring &);
  size_t colors;
  Legion::LogicalPartition color_partition;
};

template<>
struct topology_data<topology::canonical_base> {
  using type = topology::canonical_base;
  topology_data(const type::coloring &) {}
};

template<>
struct topology_data<topology::ntree_base> {
  using type = topology::ntree_base;
  topology_data(const type::coloring &) {}
};

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::unstructured_mesh_base_t> {
  using type = topology::unstructured_mesh_base_t;
  topology_data(const type::coloring &);

#if 0
  std::vector<base_data_t> entities;
  std::vector<base_data_t> adjacencies;
  std::vector<Legion::LogicalPartition> exclusive;
  std::vector<Legion::LogicalPartition> shared;
  std::vector<Legion::LogicalPartition> ghost;
  std::vector<Legion::LogicalPartition> ghost_owners;
#endif
};

#if 0
struct unstructured_mesh_dense_runtime_data_t {
}; // struct unstructured_mesh_dense_runtime_data_t

struct unstructured_mesh_ragged_runtime_data_t {
}; // struct unstructured_mesh_ragged_runtime_data_t

struct unstructured_mesh_sparse_runtime_data_t {
}; // struct unstructured_mesh_sparse_runtime_data_t

struct unstructured_mesh_subspace_runtime_data_t {
}; // struct unstructured_mesh_subspace_runtime_data_t

struct structured_mesh_runtime_data_t {
}; // struct structured_mesh_runtime_data_t
#endif

} // namespace data
} // namespace flecsi
