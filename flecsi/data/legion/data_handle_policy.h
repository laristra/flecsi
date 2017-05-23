/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_legion_data_handle_policy_h
#define flecsi_data_legion_data_handle_policy_h

#include "legion.h"

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {

  /*
  struct legion_handle_policy_t
  {
    size_t exclusive_priv;
    size_t shared_priv;
    size_t ghost_priv;
    Legion::LogicalRegion lr;
    Legion::IndexPartition exclusive_ip;
    Legion::IndexPartition shared_ip;
    Legion::IndexPartition ghost_ip;
    Legion::LogicalRegion exclusive_lr;
    Legion::LogicalRegion shared_lr;
    Legion::LogicalRegion ghost_lr;
    Legion::PhysicalRegion exclusive_pr;
    void* exclusive_data;
    Legion::PhysicalRegion shared_pr;
    void* shared_data;
    Legion::PhysicalRegion ghost_pr;
    void* ghost_data;
    Legion::Context context;
    Legion::Runtime* runtime;
    
    Legion::PhaseBarrier* pbarrier_as_master_ptr;
    std::vector<Legion::PhaseBarrier*> masters_pbarriers_ptrs;

    std::vector<Legion::PhysicalRegion> pregions_neighbors_shared;
    Legion::LogicalRegion lregion_ghost;
    size_t ghost_copy_task_id;
    Legion::FieldID ghost_fid;
    bool is_readable;
  };
  */

///
/// \class legion_data_handle_policy_t data_handle_policy.h
/// \brief legion_data_handle_policy_t provides...
///
struct legion_data_handle_policy_t
{
  using field_id_t = LegionRuntime::HighLevel::FieldID;

  field_id_t fid;
  Legion::LogicalRegion exclusive_lr;
  Legion::LogicalRegion shared_lr;
  Legion::LogicalRegion ghost_lr;
  Legion::PhaseBarrier* pbarriers_as_master;
  Legion::PhaseBarrier* ghost_owners_pbarriers;
  std::vector<Legion::LogicalRegion> ghost_owners_lregions;
  Legion::LogicalRegion color_region;
  Legion::IndexPartition primary_ghost_ip;
  Legion::IndexPartition excl_shared_ip;
}; // class legion_data_handle_policy_t

} // namespace flecsi

#endif // flecsi_data_legion_data_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
