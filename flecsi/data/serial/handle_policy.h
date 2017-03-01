/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

///
// \file serial/handle_policy.h
// \authors nickm
// \date Initial file creation: Feb 27, 2017
///

#ifndef flecsi_serial_handle_policy_h
#define flecsi_serial_handle_policy_h

#include <cstddef>

namespace flecsi {
namespace data {

struct serial_handle_policy_t
{
  void* data;
  size_t size;
};

} // namespace data
} // namespace flecsi

#endif // flecsi_serial_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
