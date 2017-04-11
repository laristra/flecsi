/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include "flecsi/execution/legion/internal_task.h"
#include "cinchtest.h"

///
/// \file
/// \date Initial file creation: Apr 01, 2017
///

namespace flecsi {
namespace execution {

// Define a Legion task to register.
int internal_task_example_1(const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context context,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime) {
} // internal_task_example

// Register the task. The task id is automatically generated.
__flecsi_internal_register_legion_task(internal_task_example_1, loc,
  single);

#if 0
// Define a Legion task to register.
int internal_task_example_2(const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context context,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime) {
} // internal_task_example

// Register the task. The task id is automatically generated.
__flecsi_internal_register_legion_task(internal_task_example_2, toc,
  single);
#endif

void driver(int argc, char ** argv) {

  // These keys will allow you to lookup the task id that was assigned.
  auto key_1 = __flecsi_task_key(internal_task_example_1, loc);
#if 0
  auto key_2 = __flecsi_internal_make_legion_task_key(internal_task_example_2,
    toc, single);
#endif

  // Lookup the task ids.
  auto tid_1 = context_t::instance().task_id(key_1);
#if 0
  auto tid_2 = context_t::instance().task_id(key_2);
#endif

  clog(info) << "Task ID: " << tid_1 << std::endl;
#if 0
  clog(info) << "Task ID: " << tid_2 << std::endl;
#endif

  ASSERT_EQ(tid_1, 1);
#if 0
  ASSERT_EQ(tid_2, 2);
#endif

} // driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
