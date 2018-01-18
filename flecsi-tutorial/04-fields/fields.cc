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

void initialize_pressure(mesh<ro> mesh, field<rw> p) {
  for(auto c: mesh.cells(owned)) {
    p(c) = double(c->id());
  } // for
} // initialize_pressure

flecsi_register_task(initialize_pressure, hydro, loc, single);

void print_pressure(mesh<ro> mesh, field<ro> p) {
  for(auto c: mesh.cells(owned)) {
    std::cout << "cell id: " << c->id() << " has pressure " <<
      p(c) << std::endl;
  } // for
} // print_pressure

flecsi_register_task(print_pressure, hydro, loc, single);

} // namespace hydro

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto p = flecsi_get_handle(m, hydro, pressure, double, dense, 0);

  flecsi_execute_task(initialize_pressure, hydro, single, m, p);
  flecsi_execute_task(print_pressure, hydro, single, m, p);

} // driver

} // namespace execution
} // namespace flecsi
