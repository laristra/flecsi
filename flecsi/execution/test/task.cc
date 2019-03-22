/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <flecsi/utils/ftest.h>

#define __FLECSI_PRIVATE__
#include <flecsi/execution/execution.h>

using namespace flecsi::execution;

flog_register_tag(task);

namespace task {

/*
  A simple task with no arguments.
 */

void
simple(int value) {
  {
    flog_tag_guard(task);
    flog(info) << "Hello World!" << std::endl;
    flog(info) << "value = " << value << std::endl;
  }
} // simple

flecsi_register_task(simple, task, loc, index);

} // namespace task

/*
  Test driver.
 */

int
test_driver(int argc, char ** argv) {

  FTEST();

  flecsi_execute_task(simple, task, index, 10);

  return 0;
}

ftest_register_test(test_driver);
