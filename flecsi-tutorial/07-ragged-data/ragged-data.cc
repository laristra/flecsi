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
flecsi_register_field(mesh_t, hydro, densities, double, ragged, 1, cells);

namespace hydro {

void initialize_materials(mesh<ro> mesh, ragged_field_mutator d) {

  for(auto c: mesh.cells(owned)) {
    const size_t random = (rand()/double{RAND_MAX}) * 5;

    d.resize(c, random);

    for(size_t i{0}; i<random; ++i) {
      d(c,i) = i;
    } // for
  } // for
} // initialize_pressure

flecsi_register_task(initialize_materials, hydro, loc, single);

void print_materials(mesh<ro> mesh, ragged_field<ro> d) {
  for(auto c: mesh.cells()) {
    for(auto m: d.entries(c)) {
      std::cout << d(c,m) << " ";
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
  auto d = flecsi_get_mutator(m, hydro, densities, double, ragged, 0, 5);

  flecsi_execute_task(initialize_materials, hydro, single, m, d);
  } // scope

  {
  auto d = flecsi_get_handle(m, hydro, densities, double, ragged, 0);

  flecsi_execute_task(print_materials, hydro, single, m, d);
  } // scope

} // driver

} // namespace execution
} // namespace flecsi
