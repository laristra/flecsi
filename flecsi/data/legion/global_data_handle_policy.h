/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <legion_stl.h>

#include <flecsi/runtime/types.h>

namespace flecsi {

/*!
 The legion_global_data_handle_policy_t type provides backend storage for
 interfacing to the Legion runtime.

 @ingroup data
 */

struct legion_global_data_handle_policy_t {
  legion_global_data_handle_policy_t() {}

  legion_global_data_handle_policy_t(
    const legion_global_data_handle_policy_t & p) = default;

  bool * ghost_is_readable;
  bool * write_phase_started;

  // +++ The following fields are set from get_handle(), reading
  // information from the context which is data that is the same
  // across multiple ranks/colors and should be used ONLY as read-only data

  field_id_t fid;
  field_id_t id_fid;
  size_t index_space;
  size_t data_client_hash;

  // These depend on color but are only used in specifying
  // the region requirements
  Legion::LogicalRegion entire_region;
  Legion::LogicalPartition color_partition;

  // Tuple-walk copies data_handle then discards updates at the end.
  // Some pointers are necessary for updates to live between walks.
  const Legion::STL::map<LegionRuntime::Arrays::coord_t,
    LegionRuntime::Arrays::coord_t> * global_to_local_color_map_ptr;

  // +++ The following fields are set on the execution side of the handle
  // inside the actual Legion task once we have the physical regions

  Legion::Context context;
  Legion::Runtime * runtime;
  Legion::PhysicalRegion color_pr;
  size_t color_priv;
}; // class legion_global_data_handle_policy_t

} // namespace flecsi
