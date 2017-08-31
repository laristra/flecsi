/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: May 4, 2017
///

#include <cinchtest.h>

#include "flecsi/execution/execution.h"
#include "flecsi/execution/future.h"


namespace flecsi {
namespace execution {

double local_value_task(
        const int my_color)
{
  return static_cast<double>(my_color);
}
flecsi_register_task(local_value_task, loc, single|leaf);


//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  int num_colors, my_color;
  MPI_Comm_size(MPI_COMM_WORLD, &num_colors);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_color);
  clog(trace) << "Rank " << my_color << " in driver" << std::endl;

  for(int cycle=1; cycle < 10; cycle++) {
    auto local_future =
      flecsi_execute_task(local_value_task, single, (my_color + 1) * cycle);

    auto global_max_future =
      flecsi::execution::context_t::instance().reduce_max(local_future);
    flecsi_future__<double> *flecsi_max_future = &global_max_future;
    const bool silence_warnings = true;
    const size_t index = 0;
    double global_max = flecsi_max_future->get(index, silence_warnings);

    auto global_min_future =
      flecsi::execution::context_t::instance().reduce_min(local_future);
    flecsi_future__<double> *flecsi_min_future = &global_min_future;
    double global_min = flecsi_min_future->get(index, silence_warnings);
 
    ASSERT_EQ(global_max, static_cast<double>(num_colors * cycle));
    ASSERT_EQ(global_min, static_cast<double>(cycle));
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
