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

struct region {
  region(std::size_t n, const runtime::context::field_info_store_t & fs)
    : index_space(run().create_index_space(ctx(), Legion::Rect<1>(0, n - 1))),
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
};

template<class Topo>
struct simple : region {
  using type = Topo;
  simple(std::size_t n = 1)
    : region(n,
        runtime::context_t::instance().get_field_info_store<Topo>(dense)) {}
};
} // namespace legion

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::global> : legion::simple<topology::global> {
  topology_data(const type::coloring &) {}
};

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::index> : legion::simple<topology::index> {
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
struct topology_data<topology::unstructured_base> {
  using type = topology::unstructured_base;
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
struct unstructured_dense_runtime_data_t {
}; // struct unstructured_dense_runtime_data_t

struct unstructured_ragged_runtime_data_t {
}; // struct unstructured_ragged_runtime_data_t

struct unstructured_sparse_runtime_data_t {
}; // struct unstructured_sparse_runtime_data_t

struct unstructured_subspace_runtime_data_t {
}; // struct unstructured_subspace_runtime_data_t

struct structured_mesh_runtime_data_t {
}; // struct structured_mesh_runtime_data_t
#endif

/*----------------------------------------------------------------------------*
  Structured Mesh Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::structured_base> {
  using type = topology::structured_base;
  topology_data(const type::coloring &);
};

} // namespace data
} // namespace flecsi
