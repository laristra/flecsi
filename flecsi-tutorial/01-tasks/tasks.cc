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

/*----------------------------------------------------------------------------*
  Documentation for this example can be found in README.md.
 *----------------------------------------------------------------------------*/

#include <iostream>

#include <flecsi/execution/execution.h>

namespace example {

// This is the definition of simple_task

void
simple_task() {

  // Print message from inside of the task

  std::cout << "Hello World from " << __FUNCTION__ << std::endl;

} // simple_task

// This line registers the "simple_task" (defined directly above)
// with the FleCSI runtime.
//
// NOTE: The task must be registered using the actual C++
// namespace in which it is defined, i.e., "example".

flecsi_register_task(simple_task, example, loc, index);

} // namespace example

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {

  // This time, the driver executes a task to do the output

  flecsi_execute_task(simple_task, example, index);

} // driver

} // namespace execution
} // namespace flecsi
