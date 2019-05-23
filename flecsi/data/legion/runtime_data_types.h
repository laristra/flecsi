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
#else
#include <flecsi/data/common/field_info.h>
#endif

#include <legion.h>

#include <unordered_map>

namespace flecsi {
namespace data {
namespace legion {

struct base_data_t {
  size_t index_space_id;
  // Legion::IndexSpace index_space;
  // Legion::FieldSpace field_space;
  Legion::LogicalRegion logical_region;
}; // base_data_t

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

struct global_runtime_data_t : public base_data_t {};

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

struct index_runtime_data_t : public base_data_t {
  size_t colors;
  Legion::LogicalPartition color_partition;
}; // struct index_runtime_data_t

struct unstructured_mesh_runtime_data_t {
  std::vector<base_data_t> entities;
  std::vector<base_data_t> adjacencies;
  std::vector<Legion::LogicalPartition> exclusive;
  std::vector<Legion::LogicalPartition> shared;
  std::vector<Legion::LogicalPartition> ghost;
  std::vector<Legion::LogicalPartition> ghost_owners;
}; // struct unstructured_mesh_runtime_data_t

struct structured_mesh_runtime_data_t : public base_data_t {
}; // struct structured_mesh_runtime_data_t

} // namespace legion
} // namespace data
} // namespace flecsi
