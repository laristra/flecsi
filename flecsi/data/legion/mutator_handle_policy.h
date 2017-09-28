/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_legion_mutator_handle_policy_h
#define flecsi_data_legion_mutator_handle_policy_h

#include <legion.h>
#include <legion_stl.h>

#include "flecsi/runtime/types.h"

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

struct legion_mutator_handle_policy_t
{
  legion_mutator_handle_policy_t(){}

  legion_mutator_handle_policy_t(const legion_mutator_handle_policy_t& p) = default;

}; // class legion_mutator_handle_policy_t

} // namespace flecsi

#endif // flecsi_data_legion_mutator_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
