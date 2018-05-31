/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/data/common/data_types.h>
#include <flecsi/runtime/types.h>

//----------------------------------------------------------------------------//
/// @file
/// @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The legion_mutator_handle_policy_t type provides backend storage for
//! interfacing to the Legion runtime.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct legion_mutator_handle_policy_t {
  legion_mutator_handle_policy_t() {}

  legion_mutator_handle_policy_t(const legion_mutator_handle_policy_t & p) =
      default;

  using offset_t = data::sparse_data_offset_t;

  field_id_t fid;
  size_t index_space;
  size_t data_client_hash;
  size_t slots;
  size_t reserve;

  // These depend on color but are only used in specifying
  // the region requirements
  Legion::LogicalRegion offsets_color_region;
  Legion::LogicalRegion offsets_exclusive_lr;
  Legion::LogicalRegion offsets_shared_lr;
  Legion::LogicalRegion offsets_ghost_lr;

  Legion::LogicalRegion entries_color_region;
  Legion::LogicalRegion entries_exclusive_lr;
  Legion::LogicalRegion entries_shared_lr;
  Legion::LogicalRegion entries_ghost_lr;

  Legion::LogicalRegion metadata_color_region;

  Legion::Context context;
  Legion::Runtime * runtime;

  Legion::PhysicalRegion offsets_exclusive_pr;
  Legion::PhysicalRegion offsets_shared_pr;
  Legion::PhysicalRegion offsets_ghost_pr;

  Legion::PhysicalRegion entries_exclusive_pr;
  Legion::PhysicalRegion entries_shared_pr;
  Legion::PhysicalRegion entries_ghost_pr;

  Legion::PhysicalRegion metadata_pr;

  void* metadata;

  offset_t* offsets;
  void* offsets_data[3];

  size_t offsets_size = 0;
  uint8_t* entries;
  size_t entries_size = 0;
  void* entries_data[3];

}; // class legion_mutator_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
