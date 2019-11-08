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

#include <flecsi/topology/base.hh>

#include <legion.h>

#include <unordered_map>

namespace flecsi {
namespace data {

namespace legion {
struct topology_base {
  ~topology_base();
  size_t index_space_id = unique_isid_t::instance().next(); // TODO: needed?
  Legion::IndexSpace index_space;
  Legion::FieldSpace field_space;
  Legion::LogicalRegion logical_region;
};
} // namespace legion

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::global_t> : legion::topology_base {
  using type = topology::global_t;
  topology_data(const type::coloring &) {}
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
