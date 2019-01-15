/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi/execution/context.h>

/*!
 @def flecsi_internal_legion_task

 This macro simplifies pure Legion task definitions by filling in the
 boiler-plate function arguments.

 @param task_name   The plain-text task name.
 @param return_type The return type of the task.

 @ingroup legion-execution
*/

#define flecsi_internal_legion_task(task_name, return_type)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Legion task template */                                                   \
  inline return_type task_name(const Legion::Task * task,                      \
    const std::vector<Legion::PhysicalRegion> & regions, Legion::Context ctx,  \
    Legion::Runtime * runtime)

namespace flecsi {
namespace execution {

/*----------------------------------------------------------------------------*
  Legion top-level task.
 *----------------------------------------------------------------------------*/

void top_level_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {

  context_t & context_ = context_t::instance();

  /*
    Initialize MPI interoperability.
   */

  context_.connect_with_mpi(ctx, runtime);
  context_.wait_on_mpi(ctx, runtime);
  
  auto args = runtime->get_input_args();
  context_.top_level_action()(args.argc, args.argv);

  /*
    Finish up Legion runtime and fall back out to MPI.
   */

  context_.unset_call_mpi(ctx, runtime);
  context_.handoff_to_mpi(ctx, runtime);
} // top_level_task

/*----------------------------------------------------------------------------*
  Other legion tasks.
 *----------------------------------------------------------------------------*/

/*!
 Interprocess communication to pass control to MPI runtime.

 @ingroup legion-execution
 */

flecsi_internal_legion_task(handoff_to_mpi_task, void) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

flecsi_internal_register_legion_task(handoff_to_mpi_task,
  processor_type_t::loc, index | leaf);

/*!
 Interprocess communication to wait for control to pass back to the Legion
 runtime.

 @ingroup legion-execution
 */

flecsi_internal_legion_task(wait_on_mpi_task, void) {
  context_t::instance().wait_on_mpi();
} // wait_on_mpi_task

flecsi_internal_register_legion_task(wait_on_mpi_task,
  processor_type_t::loc, index | leaf);

/*!
 Interprocess communication to unset mpi execute state.

 @ingroup legion-execution
*/

flecsi_internal_legion_task(unset_call_mpi_task, void) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task

flecsi_internal_register_legion_task(unset_call_mpi_task,
  processor_type_t::loc, index | leaf);

} // namespace execution
} // namespace flecsi
