/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_legion_tasks_h
#define flecsi_execution_legion_legion_tasks_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include <legion.h>

#include "flecsi/execution/legion/internal_task.h"

clog_register_tag(legion_tasks);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! @def __flecsi_internal_legion_task
//!
//! This macro simplifies pure Legion task definitions by filling in the
//! boiler-plate function arguments.
//!
//! @param task_name   The plain-text task name.
//! @param return_type The return type of the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

#define __flecsi_internal_legion_task(task_name, return_type)                  \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
/* Legion task template */                                                     \
inline return_type task_name(                                                  \
  const LegionRuntime::HighLevel::Task * task,                                 \
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,       \
  LegionRuntime::HighLevel::Context ctx,                                       \
  LegionRuntime::HighLevel::HighLevelRuntime * runtime                         \
)

//----------------------------------------------------------------------------//
//! Initial SPMD task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(spmd_task, void) {
  {
  clog_tag_guard(legion_tasks);
  clog(info) << "Executing spmd task" << std::endl;

  }

  // Add additional setup.

  // Get the input arguments from the Legion runtime
  const LegionRuntime::HighLevel::InputArgs & args =
    LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

  // Set the current task context to the driver
  context_t & context_ = context_t::instance();
  context_t::instance().push_state(utils::const_string_t{"driver"}.hash(),
    ctx, runtime, task, regions);

  const std::unordered_map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();

  {
  clog_tag_guard(legion_tasks);

  // Create sub-partitions
  for (auto itr : coloring_map) {
    clog(trace) << "key " << itr.first << std::endl;
    clog(trace) << "  primary " << std::endl;
    for (auto primary_itr = itr.second.primary.begin(); primary_itr != itr.second.primary.end(); ++primary_itr)
      clog(trace) << "    " << *primary_itr << std::endl;
    clog(trace) << "  exclusive " << std::endl;
    for (auto exclusive_itr = itr.second.exclusive.begin(); exclusive_itr != itr.second.exclusive.end(); ++exclusive_itr)
      clog(trace) << "    " << *exclusive_itr << std::endl;
    clog(trace) << "  shared " << std::endl;
    for (auto shared_itr = itr.second.shared.begin(); shared_itr != itr.second.shared.end(); ++shared_itr)
      clog(trace) << "    " << *shared_itr << std::endl;
    clog(trace) << "  ghost " << std::endl;
    for (auto ghost_itr = itr.second.ghost.begin(); ghost_itr != itr.second.ghost.end(); ++ghost_itr)
      clog(trace) << "    " << *ghost_itr << std::endl;
  }
  }

  // run default or user-defined driver 
  driver(args.argc, args.argv); 

  // Set the current task context to the driver
  context_t::instance().pop_state(utils::const_string_t{"driver"}.hash());
} // spmd_task

//----------------------------------------------------------------------------//
//! Interprocess communication to pass control to MPI runtime.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(handoff_to_mpi_task, void) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

//----------------------------------------------------------------------------//
//! Interprocess communication to wait for control to pass back to the Legion
//! runtime.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(wait_on_mpi_task, void) {
  context_t::instance().wait_on_mpi();
} // wait_on_mpi_task

//----------------------------------------------------------------------------//
//! Interprocess communication to unset mpi execute state.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(unset_call_mpi_task, void) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task

#undef __flecsi_internal_legion_task

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_legion_tasks_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
