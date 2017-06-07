/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_legion_data_handle_policy_h
#define flecsi_data_legion_data_handle_policy_h

#include "legion.h"
#include <legion_stl.h>

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {

///
/// \class legion_data_handle_policy_t data_handle_policy.h
/// \brief legion_data_handle_policy_t provides...
///
struct legion_data_handle_policy_t
{
  using field_id_t = LegionRuntime::HighLevel::FieldID;

  void copy(const legion_data_handle_policy_t& p){
      std::cout << "LDHP COPY" << std::endl;
    fid = p.fid;
    context = p.context;
    runtime = p.runtime;
    index_space = p.index_space;
    exclusive_lr = p.exclusive_lr;
    shared_lr = p.shared_lr;
    ghost_lr = p.ghost_lr;
    pbarrier_as_owner_ptr = p.pbarrier_as_owner_ptr;
    ghost_owners_pbarriers_ptrs = p.ghost_owners_pbarriers_ptrs;
    ghost_owners_lregions = p.ghost_owners_lregions;
    for(auto itr = p.global_to_local_color_map.begin(); itr !=
            p.global_to_local_color_map.end(); itr++)
        std::cout << itr->first <<
          " LDHP SRC " << itr->second << std::endl;

    global_to_local_color_map = p.global_to_local_color_map;
    color_region = p.color_region;
    primary_ghost_ip = p.primary_ghost_ip;
    excl_shared_ip = p.excl_shared_ip;
    exclusive_pr = p.exclusive_pr;
    shared_pr = p.shared_pr;
    ghost_pr = p.ghost_pr;
    ghost_is_readable = p.ghost_is_readable;
    exclusive_priv = p.exclusive_priv;
    shared_priv = p.shared_priv;
    ghost_priv = p.ghost_priv;
  }

  field_id_t fid;
  size_t index_space;
  Legion::Context context;
  Legion::Runtime* runtime;
  Legion::LogicalRegion exclusive_lr;
  Legion::LogicalRegion shared_lr;
  Legion::LogicalRegion ghost_lr;
  Legion::PhaseBarrier* pbarrier_as_owner_ptr;
  std::vector<Legion::PhaseBarrier*> ghost_owners_pbarriers_ptrs;
  std::vector<Legion::LogicalRegion> ghost_owners_lregions;
  Legion::STL::map<LegionRuntime::Arrays::coord_t,
    LegionRuntime::Arrays::coord_t> global_to_local_color_map;
  Legion::LogicalRegion color_region;
  Legion::IndexPartition primary_ghost_ip;
  Legion::IndexPartition excl_shared_ip;
  Legion::PhysicalRegion exclusive_pr;
  Legion::PhysicalRegion shared_pr;
  Legion::PhysicalRegion ghost_pr;
  size_t exclusive_priv;
  size_t shared_priv;
  size_t ghost_priv;
  bool ghost_is_readable;
}; // class legion_data_handle_policy_t

} // namespace flecsi

#endif // flecsi_data_legion_data_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
