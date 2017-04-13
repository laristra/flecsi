/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include "flecsi/execution/legion/legion_tasks.h"

#include <cinchlog.h>

#include "flecsi/execution/context.h"
#include "flecsi/utils/common.h"

namespace flecsi {
namespace execution {

__flecsi_internal_register_legion_task(spmd_task, loc, index);
__flecsi_internal_register_legion_task(handoff_to_mpi_task, loc, index | leaf);
__flecsi_internal_register_legion_task(wait_on_mpi_task, loc, index | leaf);
__flecsi_internal_register_legion_task(unset_call_mpi_task, loc, index | leaf);

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
