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

using namespace flecsi;
using namespace flecsi::tutorial;

flecsi_register_data_client(mesh_t, clients, mesh);
flecsi_register_field(mesh_t, hydro, pressure, double, dense, 1, cells);

namespace hydro {

void simple(field<rw> pressure) {
  std::cout << "Hello World from " << __FUNCTION__ << std::endl;
} // simple

flecsi_register_task(simple, hydro, loc, single);

} // namespace hydro

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  auto mesh = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto pressure = flecsi_get_handle(mesh, hydro, pressure, double, dense, 0);

  flecsi_execute_task(simple, hydro, single, pressure);

} // driver

} // namespace execution
} // namespace flecsi
