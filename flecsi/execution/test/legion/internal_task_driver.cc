/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <flecsi/execution/legion/internal_task.h>
#include "cinchtest.h"

///
/// \file
/// \date Initial file creation: Apr 01, 2017
///

namespace flecsi {
namespace execution {

// Define a Legion task to register.
int internal_task_example_1(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context context,
  Legion::Runtime * runtime) 
{
  clog(info) <<"Executing task1" <<std::endl;
  return 0;
} // internal_task_example

// Register the task. The task id is automatically generated.
flecsi_internal_register_legion_task(internal_task_example_1,
  processor_type_t::loc, single);

// Define a Legion task to register.
int internal_task_example_2(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context context,
  Legion::Runtime * runtime)
{
  clog(info) <<"Executing task2" <<std::endl;
  return 0;
} // internal_task_example

// Register the task. The task id is automatically generated.
flecsi_internal_register_legion_task(internal_task_example_2,
  processor_type_t::loc, index);

void driver(int argc, char ** argv) {

  // Lookup the task ids.
  auto tid_1 = context_t::instance().task_id<
    flecsi_internal_task_key(internal_task_example_1)>();
  auto tid_2 = context_t::instance().task_id<
    flecsi_internal_task_key(internal_task_example_2)>();

  clog(info) << "Task ID task 1: " << tid_1 << std::endl;
  clog(info) << "Task ID: task 2" << tid_2 << std::endl;

  // This will not work as new internal tasks are added
  //ASSERT_EQ(tid_1, 8);
  //ASSERT_EQ(tid_2, 9);

  auto runtime = Legion::Runtime::get_runtime();
  auto context = Legion::Runtime::get_context();  
  
  auto key_1 = flecsi_internal_task_key(internal_task_example_1);
  auto key_2 = flecsi_internal_task_key(internal_task_example_2);

  //executing legion tasks with pure legion calls
  Legion::TaskLauncher launcher(
    context_t::instance().task_id(key_1),
    Legion::TaskArgument(0,0));
  auto f = runtime->execute_task(context, launcher);

  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher index_launcher(
    context_t::instance().task_id(key_2),
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(0, 0),
    arg_map
  );
   
  //index_launcher.tag=MAPPER_FORCE_RANK_MATCH;
 auto fm = runtime->execute_index_space(context, index_launcher);
 fm.wait_all_results();
} // driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
