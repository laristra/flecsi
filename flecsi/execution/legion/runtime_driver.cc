/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Jul 26, 2016
///

#include "flecsi/execution/legion/runtime_driver.h"

#include <legion.h>

#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/legion_tasks.h"
#include "flecsi/execution/legion/mapper.h"
#include "flecsi/utils/common.h"

clog_register_tag(runtime_driver);

namespace flecsi {
namespace execution {

void runtime_driver(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::HighLevelRuntime * runtime)
  {
    {
    clog_tag_guard(runtime_driver);
    clog(info) << "In Legion runtime driver" << std::endl;
    }

    // Get the input arguments from the Legion runtime
    const Legion::InputArgs & args =
      Legion::HighLevelRuntime::get_input_args();

    // Initialize MPI Interoperability
    context_t & context_ = context_t::instance();
    context_.connect_with_mpi(ctx, runtime);
    context_.wait_on_mpi(ctx, runtime);

#if defined FLECSI_ENABLE_SPECIALIZATION_DRIVER
    {
    clog_tag_guard(runtime_driver);
    clog(info) << "Executing specialization driver task" << std::endl;
    }

    // Set the current task context to the driver
    context_.push_state(utils::const_string_t{"specialization_driver"}.hash(),
      ctx, runtime, task, regions);

    // run default or user-defined specialization driver 
    specialization_driver(args.argc, args.argv);

    // Set the current task context to the driver
    context_.pop_state( utils::const_string_t{"specialization_driver"}.hash());
#endif // FLECSI_ENABLE_SPECIALIZATION_DRIVER

  // Add reduction of meta data required to construct Legion data structures.

#if 0
  for(auto p: context_.partitions()) {
    clog_container_one(info, "exclusive", p.second.exclusive, clog::space);
  } // for
#endif

  // Register user data
  //data::storage_t::instance().register_all();

  // Must epoch launch
  Legion::MustEpochLauncher must_epoch_launcher;

  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "MPI size is " << size << std::endl;
  }

#if 0
  auto spmd_id = context_.task_id(__flecsi_task_key(spmd_task, loc));

  // Add colors to must_epoch_launcher
  for(size_t i(0); i<size; ++i) {
    Legion::TaskLauncher spmd_launcher(spmd_id, Legion::TaskArgument(0, 0));
    spmd_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    Legion::DomainPoint point(i);
    must_epoch_launcher.add_single_task(point, spmd_launcher);
  } // for

  // Launch the spmd tasks
  auto future = runtime->execute_must_epoch(ctx, must_epoch_launcher);
  future.wait_all_results();
#endif

  // Finish up Legion runtime and fall back out to MPI.
  context_.unset_call_mpi(ctx, runtime);
  context_.handoff_to_mpi(ctx, runtime);
  } // runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
