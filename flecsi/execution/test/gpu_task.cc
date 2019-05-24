/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_task_driver_h
#define flecsi_task_driver_h

#include <cinchtest.h>
#include <iostream>

#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>
#include <flecsi/utils/common.h>

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Task registration.
//----------------------------------------------------------------------------//

void
gpu_task_single(void) {
  clog(info) << "This is a single gpu task" << std::endl;
}

flecsi_register_task(gpu_task_single, flecsi::execution, toc, single);


void
gpu_task_index(void) {
  clog(info) << "This is an index gpu task" << std::endl;
}

flecsi_register_task(gpu_task_index, flecsi::execution, toc, index);
//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {

  clog(info) << "Inside user driver" << std::endl;

  auto f1 = flecsi_execute_task(gpu_task_single, flecsi::execution, single);

  f1.wait();

  auto f2 = flecsi_execute_task(gpu_task_index, flecsi::execution, index);

  f2.wait();

} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(simple_task, testname) {} // TEST

} // namespace execution
} // namespace flecsi

#endif // flecsi_task_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
