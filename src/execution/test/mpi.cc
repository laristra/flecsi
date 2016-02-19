/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/execution/mpi_execution_policy.h"
#include "flecsi/execution/task.h"

using execution_t = flecsi::execution_t<flecsi::mpi_execution_policy_t>;
using return_type_t = execution_t::return_type_t;

return_type_t world_size() {
  int world_size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  return 0;
}

#define execute(task, ...) \
  execution_t::execute_task(task, ##__VA_ARGS__)

TEST(task, execute) {
  ASSERT_LT(execute(world_size), 1);
} // TEST
