/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include "flecsi/execution/legion/legion_tasks.h"

#include <cinchlog.h>

#include "flecsi/execution/context.h"
#include "flecsi/utils/common.h"

clog_register_tag(legion_tasks);

namespace flecsi {
namespace execution {

#if 0
void spmd_task(const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    {
    clog_tag_guard(legion_tasks);
    clog(info) << "Executing driver task" << std::endl;
    }

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
#endif

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
