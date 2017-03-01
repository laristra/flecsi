/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
// \file legion/runtime_driver.cc
// \authors bergen
// \date Initial file creation: Jul 26, 2016
///

#include "flecsi/execution/legion/runtime_driver.h"

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"

#ifndef FLECSI_DRIVER
  #include "flecsi/execution/default_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_DRIVER)
#endif

#ifndef FLECSI_SPECIALIZATION_DRIVER
  #include "flecsi/execution/default_specialization_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_SPECIALIZATION_DRIVER)
#endif

namespace flecsi {
namespace execution {

void legion_runtime_driver(const LegionRuntime::HighLevel::Task * task,
	const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
	LegionRuntime::HighLevel::Context ctx,
	LegionRuntime::HighLevel::HighLevelRuntime * runtime)
	{
    const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

    // Set the current task context to the driver
		context_t::instance().push_state(utils::const_string_t{"driver"}.hash(),
      ctx, runtime, task, regions);

    // run default or user-defined specialization driver 
    specialization_driver(args.argc, args.argv);

    // run default or user-defined driver 
    driver(args.argc, args.argv); 

    // Set the current task context to the driver
		context_t::instance().pop_state(utils::const_string_t{"driver"}.hash());

	} // legion_runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
