/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/runtime/types.h>

//----------------------------------------------------------------------------//
/// @file
/// @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The legion_sparse_data_handle_policy_t type provides backend storage for
//! interfacing to the Legion runtime.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct legion_sparse_data_handle_policy_t {
  legion_sparse_data_handle_policy_t() {}

  legion_sparse_data_handle_policy_t(
      const legion_sparse_data_handle_policy_t & p) = default;

  bool * ghost_is_readable;
  bool * write_phase_started;

  // +++ The following fields are set from get_handle(), reading
  // information from the context which is data that is the same
  // across multiple ranks/colors and should be used ONLY as read-only data

  field_id_t fid;
  size_t index_space;
  size_t data_client_hash;

  size_t reserve;
  size_t max_entries_per_index;

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

  Legion::PhysicalRegion metadata_pr;

  std::vector<Legion::LogicalRegion> ghost_owners_offsets_lregions;
  std::vector<Legion::LogicalRegion> ghost_owners_offsets_subregions;

  std::vector<Legion::LogicalRegion> ghost_owners_entries_lregions;

  // Tuple-walk copies data_handle then discards updates at the end.
  // Some pointers are necessary for updates to live between walks.
  Legion::PhaseBarrier * pbarrier_as_owner_ptr;
  std::vector<Legion::PhaseBarrier *> ghost_owners_pbarriers_ptrs;
  const Legion::STL::map<
      LegionRuntime::Arrays::coord_t,
      LegionRuntime::Arrays::coord_t> * global_to_local_color_map_ptr;

  // +++ The following fields are set on the execution side of the handle
  // inside the actual Legion task once we have the physical regions

  size_t exclusive_priv;
  size_t shared_priv;
  size_t ghost_priv;

  void* metadata;

  size_t offsets_size = 0;
  size_t entries_size = 0;

  void* entries_data[3];
}; // class legion_sparse_data_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
