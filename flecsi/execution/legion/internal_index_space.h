/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_internal_index_space_h
#define flecsi_execution_legion_internal_index_space_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 22, 2017
//----------------------------------------------------------------------------//


namespace flecsi{
namespace execution{

/// If more than 4096 internal fields are allocated, storage_policy
/// unique_fid_t must be updated
enum internal_index_space
{
  global_is = 4096,
  color_is,
};

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_legion_internal_index_space_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
