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

int my_color_task(
        const int my_color)
{
  clog(error) << "Rank " << my_color << " in my_color_task" << std::endl;
  return my_color;
}
flecsi_register_task(my_color_task, flecsi::loc, flecsi::single);

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
  const int my_color = runtime->find_local_MPI_rank();
  clog(error) << "Rank " << my_color << " in driver" << std::endl;

  legion_future__<int> future =
      flecsi_execute_task(my_color_task, single, my_color);

  clog(error) << "Rank " << my_color << " get future " << future.get() << std::endl;

} // driver

} // namespace execution
} // namespace flecsi

TEST(reduction, testname) {

} // TEST


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
