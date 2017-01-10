/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_permissions_h
#define flecsi_data_permissions_h

///
/// \file
/// \date Initial file creation: Sep 07, 2016
///

namespace flecsi {
namespace data {

///
/// \enum permissions_t
///
enum permissions_t : size_t
{
  read,
  read_write,
  write,
  reduce
}; // enum permissions_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_permissions_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
