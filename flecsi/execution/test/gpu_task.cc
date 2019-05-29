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
gpu_task(void) {
  clog(info) << "This is a gpu task" << std::endl;
}

flecsi_register_task(gpu_task, flecsi::execution, toc, single);

//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {

  clog(info) << "Inside user driver" << std::endl;

  auto f = flecsi_execute_task(gpu_task, flecsi::execution, single);

  f.wait();

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
