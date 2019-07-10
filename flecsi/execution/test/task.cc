/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#include <flecsi/utils/ftest.hh>

#define __FLECSI_PRIVATE__
#include <flecsi/execution/common/launch.hh>
#include <flecsi/execution/execution.hh>

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

template<class T>
void
simple2(T t) {
  flog_tag_guard(task);
  flog(info) << "simple2(" << t << ")\n";
}

flecsi_register_task(simple, task, loc, index);

} // namespace task

/*
  Test driver.
 */

int
test_driver(int argc, char ** argv) {

  FTEST();

  flecsi_execute_task(simple, task, single, 10);

  flecsi_execute_task(simple, task, index, 8);

  task_interface_t::execute<task::simple2<float>>(6.1);

  return FTEST_RESULT();
}

ftest_register_driver(test_driver);
