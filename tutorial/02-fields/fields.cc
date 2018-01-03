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

#include <iostream>

#include<flecsi-tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>

using namespace flecsi::tutorial;

namespace hydro {

flecsi_register_field(mesh_t, hydro, pressure, double, dense, 1, cells);

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
