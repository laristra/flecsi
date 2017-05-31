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
  using field_id_t = Legion::FieldID;

  void copy(const legion_data_handle_policy_t& p){
    fid = p.fid;
    index_space = p.index_space;
    exclusive_lr = p.exclusive_lr;
    shared_lr = p.shared_lr;
    ghost_lr = p.ghost_lr;
    pbarrier_as_owner_ptr = p.pbarrier_as_owner_ptr;
    ghost_owners_pbarriers_ptrs = p.ghost_owners_pbarriers_ptrs;
    ghost_owners_lregions = p.ghost_owners_lregions;
    color_region = p.color_region;
    primary_ghost_ip = p.primary_ghost_ip;
    excl_shared_ip = p.excl_shared_ip;
    exclusive_pr = p.exclusive_pr;
    shared_pr = p.shared_pr;
    ghost_pr = p.ghost_pr;
    ghost_is_readable = p.ghost_is_readable;
  }

  field_id_t fid;
  size_t index_space;
  Legion::LogicalRegion exclusive_lr;
  Legion::LogicalRegion shared_lr;
  Legion::LogicalRegion ghost_lr;
  Legion::PhaseBarrier* pbarrier_as_owner_ptr;
  std::vector<Legion::PhaseBarrier*> ghost_owners_pbarriers_ptrs;
  std::vector<Legion::LogicalRegion> ghost_owners_lregions;
  Legion::LogicalRegion color_region;
  Legion::IndexPartition primary_ghost_ip;
  Legion::IndexPartition excl_shared_ip;
  Legion::PhysicalRegion exclusive_pr;
  Legion::PhysicalRegion shared_pr;
  Legion::PhysicalRegion ghost_pr;
  bool ghost_is_readable;
}; // class legion_data_handle_policy_t

} // namespace flecsi

#endif // flecsi_data_legion_data_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
