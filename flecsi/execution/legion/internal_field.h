/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_internal_field_h
#define flecsi_execution_legion_internal_field_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 22, 2017
//----------------------------------------------------------------------------//

#include <legion.h>

namespace flecsi{
namespace execution{

/// If more than 4096 internal fields are allocated, storage_policy
/// unique_fid_t must be updated
enum class internal_field : Legion::FieldID{
  ghost_owner_pos = (size_t(1) << 20) - 4095,
  adjacency_pos_start,
  adjacency_pos_end = adjacency_pos_start + 100,
  adjacency_index,
  adjacency_offset,
  entity_data_start,  
  entity_data_end = entity_data_start + 10  
};

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_legion_internal_field_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
