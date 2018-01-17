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

#include <cstdlib>
#include <iostream>

#include<flecsi-tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

flecsi_register_data_client(mesh_t, clients, mesh);
flecsi_register_field(mesh_t, hydro, pressure, double, sparse, 1, cells);

namespace hydro {

void initialize_materials(mesh<ro> mesh, sparse_field_mutator p) {

  for(auto c: mesh.cells(owned)) {
    const size_t random = rand()/double(RAND_MAX) * 5;

    for(size_t i{0}; i<random; ++i) {
      p(c,i) = i;
    } // for
  } // for
} // initialize_pressure

flecsi_register_task(initialize_materials, hydro, loc, single);

void print_materials(mesh<ro> mesh, sparse_field<ro> p) {
  for(auto i: p.indices()) {
    for(auto e: p.entries(i)) {
      std::cout << p(i,e) << " ";
    } // for
    std::cout << std::endl;
  } // for
} // print_pressure

flecsi_register_task(print_materials, hydro, loc, single);

} // namespace hydro

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

  {
  auto p = flecsi_get_mutator(m, hydro, pressure, double, sparse, 0, 5);

  flecsi_execute_task(initialize_materials, hydro, single, m, p);
  } // scope

  {
  auto p = flecsi_get_handle(m, hydro, pressure, double, sparse, 0);

  flecsi_execute_task(print_materials, hydro, single, m, p);
  } // scope

} // driver

} // namespace execution
} // namespace flecsi
