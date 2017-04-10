/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Jul 26, 2016
///

#include <iostream>

#include "flecsi/execution/legion/context_policy.h"
#include "flecsi/data/storage.h"

namespace flecsi {
namespace execution {

thread_local std::unordered_map<size_t,
  std::stack<std::shared_ptr<legion_runtime_state_t>>> state_;  

int
legion_context_policy_t::initialize(
  int argc,
  char ** argv
)
{
  using namespace LegionRuntime::HighLevel;

  // Register top-level task
  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  HighLevelRuntime::register_legion_task<runtime_driver>(
    TOP_LEVEL_TASK_ID, lr_loc, true, false);

  // Register user data
  data::storage_t::instance().register_all();

  // Register user tasks
  for(auto t: task_registry_) {

    // Iterate over task variants
    for(auto v: t.second) {
      v.second.second(v.second.first);
    } // for
  } // for

  // Start the runtime
  return HighLevelRuntime::start(argc, argv);
}

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
