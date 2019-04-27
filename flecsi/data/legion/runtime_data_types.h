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
  size_t id;
  Legion::IndexSpace index_space;
  Legion::FieldSpace field_space;
  Legion::LogicalRegion logical_region;
}; // isbase_t

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

struct global_runtime_data_t : public base_data_t {};

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

struct color_runtime_data_t : public base_data_t {
  Legion::IndexPartition color_partition;
}; // struct color_runtime_data_t

} // namespace legion
} // namespace data
} // namespace flecsi
