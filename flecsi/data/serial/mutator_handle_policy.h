/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_serial_mutator_handle_policy_h
#define flecsi_data_serial_mutator_handle_policy_h

//----------------------------------------------------------------------------//
/// @file
/// @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The serial_mutator_handle_policy_t type provides backend storage for
//! interfacing to the Legion runtime.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct serial_mutator_handle_policy_t
{
  serial_mutator_handle_policy_t(){}

  serial_mutator_handle_policy_t(const serial_mutator_handle_policy_t& p) = default;

}; // class serial_mutator_handle_policy_t

} // namespace flecsi

#endif // flecsi_data_serial_mutator_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
