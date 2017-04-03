/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Jul 26, 2016
///

#include "flecsi/execution/rf_mpilegion/runtime_driver.h"

#include "flecsi/execution/context.h"
#include "flecsi/utils/common.h"

namespace flecsi {
namespace execution {

void rf_mpilegion_runtime_driver(const LegionRuntime::HighLevel::Task * task,
	const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
	LegionRuntime::HighLevel::Context ctx,
	LegionRuntime::HighLevel::HighLevelRuntime * runtime)
	{
    const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

#if defined FLECSI_OVERRIDE_DEFAULT_SPECIALIZATION_DRIVER
    // Set the current task context to the driver
		context_t::instance().push_state(
      utils::const_string_t{"specialization_driver"}.hash(),
      ctx, runtime, task, regions);

    // run default or user-defined specialization driver 
    specialization_driver(args.argc, args.argv);

    // Set the current task context to the driver
		context_t::instance().pop_state(
      utils::const_string_t{"specialization_driver"}.hash());

#endif // FLECSI_OVERRIDE_DEFAULT_SPECIALIZATION_DRIVER

    // Set the current task context to the driver
		context_t::instance().push_state(utils::const_string_t{"driver"}.hash(),
      ctx, runtime, task, regions);

    // run default or user-defined driver 
    driver(args.argc, args.argv); 

    // Set the current task context to the driver
		context_t::instance().pop_state(utils::const_string_t{"driver"}.hash());

	} // rf_mpilegion_runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
