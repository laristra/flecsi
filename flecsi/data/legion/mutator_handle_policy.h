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
#include <flecsi/execution/context.h>

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

  legion_mutator_handle_policy_t(
    const legion_mutator_handle_policy_t & p) = default;

  using offset_t = data::sparse_data_offset_t;

  bool * ghost_is_readable;
  bool * write_phase_started;

  field_id_t fid;
  size_t index_space;
  size_t data_client_hash;
  size_t slots;
  size_t reserve;

  // These depend on color but are only used in specifying
  // the region requirements
  Legion::LogicalRegion entries_entire_region;
  Legion::LogicalPartition entries_color_parition;
  Legion::LogicalPartition entries_exclusive_lp;
  Legion::LogicalPartition entries_shared_lp;
  Legion::LogicalPartition entries_ghost_lp;

  Legion::LogicalRegion offsets_entire_region;                                    Legion::LogicalPartition offsets_color_partition;                               Legion::LogicalPartition offsets_exclusive_lp;
  Legion::LogicalPartition offsets_shared_lp;
  Legion::LogicalPartition offsets_ghost_lp;

  Legion::LogicalRegion metadata_entire_region;
  Legion::LogicalPartition metadata_lp;
 
  void * metadata; 

  Legion::LogicalPartition ghost_owners_offsets_lp;
//  std::vector<Legion::LogicalRegion> ghost_owners_offsets_subregions;

  Legion::LogicalPartition ghost_owners_entries_lp;


  Legion::Context context;
  Legion::Runtime * runtime;

  const Legion::STL::map<
      LegionRuntime::Arrays::coord_t,
      LegionRuntime::Arrays::coord_t> * global_to_local_color_map_ptr;

  // +++ The following fields are set on the execution side of the handle
  // inside the actual Legion task once we have the physical regions

  offset_t * offsets;
  void * offsets_data[3];

  size_t offsets_size = 0;
  uint8_t * entries;
  size_t entries_size = 0;
  void * entries_data[3];

}; // class legion_mutator_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
