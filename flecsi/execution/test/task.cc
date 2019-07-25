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
#include <flecsi/execution/execution.hh>

#if 0
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

template<class T>
void
simple2(T t) {
  flog_tag_guard(task);
  flog(info) << "simple2(" << t << ")\n";
}

} // namespace task

#endif

int
test_driver(int argc, char ** argv) {

  FTEST();

//  flecsi_execute_task(simple, task, single, 10);

//  flecsi_execute_task(simple, task, index, 8);

//  task_interface_t::execute<task::simple2<float>>(6.1);
//  flecsi::execution::execute<task::simple2<float>, loc, index>(6.2);

  return FTEST_RESULT();
}

ftest_register_driver(test_driver);
