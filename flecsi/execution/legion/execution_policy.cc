/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 12, 2017
//----------------------------------------------------------------------------//

#include "flecsi/execution/legion/execution_policy.h"
#include "flecsi/execution/task.h"

namespace flecsi {
namespace execution {

legion_execution_policy_t::runtime_state_t &
legion_execution_policy_t::runtime_state(void * task)
{
  return reinterpret_cast<flecsi::execution::base_task_t *>
    (task)->runtime_state_;
} // legion_execution_policy_t::runtime_state

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
