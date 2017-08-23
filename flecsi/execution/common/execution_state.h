/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_execution_state_h
#define flecsi_data_execution_state_h

#include <bitset>
//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

  enum execution_state_t : size_t {
    SPECIALIZATION_TLT_INIT,
    SPECIALIZATION_SPMD_INIT,
    DRIVER
  };

} // namespace flecsi

#endif // flecsi_data_execution_state_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
