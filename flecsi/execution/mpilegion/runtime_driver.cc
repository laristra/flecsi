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
		context_t::instance().set_state(ctx, runtime, task, regions);

    const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();
 
    context_t::instance().InteropHelper.connect_with_mpi(
         context_t::instance().context(), context_t::instance().runtime());

    driver(args.argc, args.argv); 

    context_t::instance().InteropHelper.call_mpi=false;
    context_t::instance().InteropHelper.handoff_to_mpi(
           context_t::instance().context(), context_t::instance().runtime());
	} // mpilegion_runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
