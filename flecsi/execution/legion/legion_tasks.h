/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_legion_tasks_h
#define flecsi_execution_legion_legion_tasks_h

///
/// \file
/// \date Initial file creation: Jul 26, 2016
///

#include <legion.h>

#include "flecsi/execution/legion/internal_task.h"

clog_register_tag(legion_tasks);

namespace flecsi {
namespace execution {

/// Avoid having to repeat all of the Legion boiler-plate function arguments.
#define legion_task(task_name, return_type)                                    \
inline return_type task_name(                                                  \
  const LegionRuntime::HighLevel::Task * task,                                 \
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,       \
  LegionRuntime::HighLevel::Context ctx,                                       \
  LegionRuntime::HighLevel::HighLevelRuntime * runtime                         \
)

//----------------------------------------------------------------------------//
// Initial SPMD task.
//----------------------------------------------------------------------------//

legion_task(spmd_task, void) {
  {
  clog_tag_guard(legion_tasks);
  clog(info) << "Executing driver task" << std::endl;
  }

  // Add additional setup.

  // Get the input arguments from the Legion runtime
  const LegionRuntime::HighLevel::InputArgs & args =
    LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

  // Set the current task context to the driver
  context_t & context_ = context_t::instance();
  context_t::instance().push_state(utils::const_string_t{"driver"}.hash(),
    ctx, runtime, task, regions);

  // run default or user-defined driver 
  driver(args.argc, args.argv); 

  // Set the current task context to the driver
  context_t::instance().pop_state(utils::const_string_t{"driver"}.hash());
} // spmd_task

//----------------------------------------------------------------------------//
// Interprocess communication to pass control to MPI runtime.
//----------------------------------------------------------------------------//

legion_task(handoff_to_mpi_task, void) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

//----------------------------------------------------------------------------//
// Interprocess communication to unset mpi execute state.
//----------------------------------------------------------------------------//

legion_task(wait_on_mpi_task, void) {
  context_t::instance().wait_on_mpi();
} // wait_on_mpi_task

//----------------------------------------------------------------------------//
// Interprocess communication to unset mpi execute state.
//----------------------------------------------------------------------------//

legion_task(unset_call_mpi_task, void) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task

#undef legion_task

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_legion_tasks_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
