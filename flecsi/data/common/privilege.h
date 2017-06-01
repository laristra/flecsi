/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_privilege_h
#define flecsi_data_privilege_h

#include <bitset>
//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

  enum privilege_t : size_t{
    dno = 0,
    dro = 1,
    dwd = 2,
    drw = 3
  };

} // namespace flecsi

#endif // flecsi_data_privilege_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
