/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: May 4, 2017
///

#include <cinchlog.h>
#include <cinchtest.h>

#include "flecsi/execution/execution.h"

clog_register_tag(reduction);

double local_value_task(
        const int my_color)
{
  return static_cast<double>(my_color);
}
flecsi_register_task(local_value_task, loc, single);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(trace) << "In specialization top-level-task init" << std::endl;
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  auto runtime = Legion::Runtime::get_runtime();
  auto ctx = Legion::Runtime::get_context();
  Legion::DynamicCollective& dc_reduction = context_t::instance().max_reduction();

  int num_colors;
  MPI_Comm_size(MPI_COMM_WORLD, &num_colors);
  const int my_color = runtime->find_local_MPI_rank();
  clog(trace) << "Rank " << my_color << " in driver" << std::endl;

  for(int cycle=1; cycle < 10; cycle++) {
    legion_future__<double> local_future =
      flecsi_execute_task(local_value_task, single, (my_color + 1) * cycle);

    local_future.defer_dynamic_collective_arrival(runtime, ctx, dc_reduction);
    dc_reduction = runtime->advance_dynamic_collective(ctx, dc_reduction);
    Legion::Future global_future = runtime->get_dynamic_collective_result(ctx,
      dc_reduction);

    double global_max = global_future.get_result<double>();
    clog(trace) << "Rank " << my_color << " get future " << local_future.get()
      << " of " << global_future.get_result<double>() <<
      " vs " << num_colors * cycle << std::endl;

    assert(global_max == static_cast<double>(num_colors * cycle));
  } // cycle

} // driver

} // namespace execution
} // namespace flecsi

TEST(reduction, testname) {

} // TEST


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
