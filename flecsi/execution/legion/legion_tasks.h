/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_legion_tasks_h
#define flecsi_execution_legion_legion_tasks_h

#include <legion.h>

#include "flecsi/execution/legion/internal_task.h"

///
/// \file
/// \date Initial file creation: Jul 26, 2016
///

namespace flecsi {
namespace execution {

/// Avoid having to repeat all of the Legion boiler-plate function arguments.
#define legion_task(task, return_type)                                         \
inline return_type task(                                                       \
  const LegionRuntime::HighLevel::Task * task,                                 \
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,       \
  LegionRuntime::HighLevel::Context ctx,                                       \
  LegionRuntime::HighLevel::HighLevelRuntime * runtime                         \
)

/// Initial SPMD task.
void spmd_task(
  const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime
);

__flecsi_internal_register_legion_task(spmd_task, loc,
  false/*single*/, true/*index*/);

/// Interprocess communication to pass control to MPI runtime.
legion_task(handoff_to_mpi_task, void) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

__flecsi_internal_register_legion_task(handoff_to_mpi_task, loc,
  false/*single*/, true/*index*/, true/*leaf*/);

/// Interprocess communication to unset mpi execute state.
legion_task(wait_on_mpi_task, void) {
  context_t::instance().wait_on_mpi();
} // wait_on_mpi_task

__flecsi_internal_register_legion_task(wait_on_mpi_task, loc,
  false/*single*/, true/*index*/, true/*leaf*/);

/// Interprocess communication to unset mpi execute state.
legion_task(unset_call_mpi_task, void) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task

__flecsi_internal_register_legion_task(unset_call_mpi_task, loc,
  false/*single*/, true/*index*/, true/*leaf*/);

#undef legion_task

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_legion_tasks_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
