/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/execution/context.h>
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

  // These depend on color but are only used in specifying
  // the region requirements
  Legion::LogicalRegion entire_region;
  Legion::LogicalPartition color_partition;
  Legion::LogicalPartition exclusive_lp;
  Legion::LogicalPartition shared_lp;
  Legion::LogicalPartition ghost_lp;
  Legion::LogicalPartition ghost_owners_lp;

  Legion::LogicalRegion metadata_entire_region;
  Legion::LogicalPartition metadata_lp;

  void * metadata;

  Legion::Context context;
  Legion::Runtime * runtime;
}; // class legion_sparse_data_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
