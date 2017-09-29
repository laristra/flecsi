/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_legion_data_handle_policy_h
#define flecsi_data_legion_data_handle_policy_h

#include <legion.h>
#include <legion_stl.h>

#include "flecsi/runtime/types.h"

//----------------------------------------------------------------------------//
/// @file
/// @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The legion_data_handle_policy_t type provides backend storage for
//! interfacing to the Legion runtime.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct legion_data_handle_policy_t
{
  legion_data_handle_policy_t(){}

  legion_data_handle_policy_t(const legion_data_handle_policy_t& p) = default;

  bool* ghost_is_readable;
  bool* write_phase_started;

  // +++ The following fields are set from get_handle(), reading
  // information from the context which is data that is the same
  // across multiple ranks/colors and should be used ONLY as read-only data

  field_id_t fid;
  field_id_t id_fid;
  size_t index_space;
  size_t data_client_hash;

  // These depend on color but are only used in specifying
  // the region requirements
  Legion::LogicalRegion color_region;
  Legion::LogicalRegion exclusive_lr;
  Legion::LogicalRegion shared_lr;
  Legion::LogicalRegion ghost_lr;
  std::vector<Legion::LogicalRegion> ghost_owners_lregions;
  std::vector<Legion::LogicalRegion> ghost_owners_subregions;

  // Tuple-walk copies data_handle then discards updates at the end.
  // Some pointers are necessary for updates to live between walks.
  Legion::PhaseBarrier* pbarrier_as_owner_ptr;
  std::vector<Legion::PhaseBarrier*> ghost_owners_pbarriers_ptrs;
  const Legion::STL::map<LegionRuntime::Arrays::coord_t,
    LegionRuntime::Arrays::coord_t>* global_to_local_color_map_ptr;

  // +++ The following fields are set on the execution side of the handle
  // inside the actual Legion task once we have the physical regions

  Legion::Context context;
  Legion::Runtime* runtime;
  Legion::PhysicalRegion exclusive_pr;
  Legion::PhysicalRegion shared_pr;
  Legion::PhysicalRegion ghost_pr;
  size_t exclusive_priv;
  size_t shared_priv;
  size_t ghost_priv;
}; // class legion_data_handle_policy_t

} // namespace flecsi

#endif // flecsi_data_legion_data_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
