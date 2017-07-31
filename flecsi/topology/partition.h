/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_partition_h
#define flecsi_partition_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

  enum partition_t : size_t{
    ex = 0x001b,
    sh = 0x010b,
    gh = 0x100b,
    pr = 0x011b,
    pg = 0x111b
  };

} // namespace flecsi

#endif // flecsi_partition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
