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

// Define a Legion task to register.
int internal_task_example_2(const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context context,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime) {
} // internal_task_example

// Register the task. The task id is automatically generated.
__flecsi_internal_register_legion_task(internal_task_example_2, toc,
  single);

void driver(int argc, char ** argv) {

  // These keys will allow you to lookup the task id that was assigned.
  auto key_1 = __flecsi_internal_task_key(internal_task_example_1, loc);
  auto key_2 = __flecsi_internal_task_key(internal_task_example_2, toc);

  // Lookup the task ids.
  auto tid_1 = context_t::instance().task_id(key_1);
  auto tid_2 = context_t::instance().task_id(key_2);

  clog(info) << "Task ID: " << tid_1 << std::endl;
  clog(info) << "Task ID: " << tid_2 << std::endl;

  ASSERT_EQ(tid_1, 1);
  ASSERT_EQ(tid_2, 2);

} // driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
