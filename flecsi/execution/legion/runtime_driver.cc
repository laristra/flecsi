/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Jul 26, 2016
///

#include "flecsi/execution/legion/runtime_driver.h"

#include "flecsi/execution/context.h"
#include "flecsi/utils/common.h"

clog_register_tag(runtime_driver);

namespace flecsi {
namespace execution {

void runtime_driver(const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    {
    clog_tag_guard(runtime_driver);
    clog(info) << "Executing Legion runtime driver" << std::endl;
    }

    // Get the input arguments from the Legion runtime
    const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

    // Initialize MPI Interoperability
    context_t & context_ = context_t::instance();
    //context_.interop_helper_.connect_with_mpi(ctx, runtime);
    //context_.interop_helper_.wait_on_mpi(ctx, runtime);

#if defined FLECSI_ENABLE_SPECIALIZATION_DRIVER
    {
    clog_tag_guard(runtime_driver);
    clog(info) << "Executing specialization driver task" << std::endl;
    }

    // Set the current task context to the driver
    context_t::instance().push_state(
      utils::const_string_t{"specialization_driver"}.hash(),
      ctx, runtime, task, regions);

    // run default or user-defined specialization driver 
    specialization_driver(args.argc, args.argv);

    // Set the current task context to the driver
    context_t::instance().pop_state(
      utils::const_string_t{"specialization_driver"}.hash());
#endif // FLECSI_ENABLE_SPECIALIZATION_DRIVER

  // Register user data
  //data::storage_t::instance().register_all();

  // Must epoch launch
  LegionRuntime::HighLevel::MustEpochLauncher must_epoch_launcher;

  int num_colors;
  MPI_Comm_size(MPI_COMM_WORLD, &num_colors);
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "MPI rank is " << num_colors << std::endl;
  }

  } // runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
