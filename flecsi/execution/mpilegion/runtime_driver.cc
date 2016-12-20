/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

/*!
 * \file mpilegion/runtime_driver.cc
 * \authors bergen
 * \date Initial file creation: Jul 26, 2016
 */

#include "flecsi/execution/mpilegion/runtime_driver.h"
#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"

#ifndef FLECSI_DRIVER
  #include "flecsi/execution/default_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_DRIVER)
#endif

namespace flecsi {
namespace execution {

void mpilegion_runtime_driver(const LegionRuntime::HighLevel::Task * task,
	const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
	LegionRuntime::HighLevel::Context ctx,
	LegionRuntime::HighLevel::HighLevelRuntime * runtime)
	{
    std::cout << "mpilegion_runtime_driver started" << std::endl;
                
    context_t & context_ = context_t::instance();
    context_.push_state(utils:const_string_t{"driver"}.hash(),
      ctx, runtime, task, regions);

    const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

    // connect legion with MPI
    context_.interop_helper_.connect_with_mpi(ctx, runtime);
    context_.interop_helper_.wait_on_mpi(ctx, runtime);
    // run default or user-defined driver 
    driver(args.argc, args.argv); 

    // finish up legion runtime and handoff to mpi
    context_.interop_helper_.unset_call_mpi(ctx, runtime);
    context_.interop_helper_.handoff_to_mpi(ctx, runtime);

    // Set the current task context to the driver
    context_t::instance().pop_state(utils:const_string_t{"driver"}.hash());
  } // mpilegion_runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
