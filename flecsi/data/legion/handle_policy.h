/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_handle_policy_h
#define flecsi_legion_handle_policy_h

#include "legion.h"

namespace flecsi {
namespace data {

class legion_handle_policy_t
{
public:
  Legion::LogicalRegion lr;
private:

};

} // namespace data
} // namespace flecsi

#endif // flecsi_legion_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
