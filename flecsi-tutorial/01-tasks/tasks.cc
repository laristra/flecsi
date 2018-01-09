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
/*! @file */

#include <iostream>

#include<flecsi/execution/execution.h>

namespace hydro {

/*!

 */

void simple() {
  std::cout << "Hello World from " << __FUNCTION__ << std::endl;
} // simple

flecsi_register_task(simple, hydro, loc, single);

} // namespace hydro

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {
  flecsi_execute_task(simple, hydro, single);
} // driver

} // namespace execution
} // namespace flecsi
