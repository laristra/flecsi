/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

///
// \file legion/handle_policy.h
// \authors nickm
// \date Initial file creation: Feb 27, 2017
///

#ifndef flecsi_legion_handle_policy_h
#define flecsi_legion_handle_policy_h

#include "legion.h"

namespace flecsi {
namespace data {

struct legion_handle_policy_t
{
  Legion::LogicalRegion lr;
  Legion::IndexPartition exclusive_ip;
  Legion::IndexPartition shared_ip;
  Legion::IndexPartition ghost_ip;
  Legion::LogicalRegion exclusive_lr;
  Legion::LogicalRegion shared_lr;
  Legion::LogicalRegion ghost_lr;
  void* exclusive_data;
  size_t exclusive_size;
  void* shared_data;
  size_t shared_size;
  void* ghost_data;
  size_t ghost_size;
};

} // namespace data
} // namespace flecsi

#endif // flecsi_legion_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
