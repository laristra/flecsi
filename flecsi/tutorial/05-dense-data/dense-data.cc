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

#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>
#include <flecsi/tutorial/specialization/mesh/mesh.h>

#include "types.h"

using namespace flecsi;
using namespace flecsi::tutorial;
using namespace types;

flecsi_register_field(mesh_t, types, f, struct_type_t, dense, 1, cells);

namespace example {

// This task initializes the field of struct_type_t data. Notice that
// nothing has changed about the iteration logic over the mesh. The only
// difference is that now the dereferenced values are struct instances.

void
initialize_field(mesh<ro> mesh, struct_field<rw> f) {
  for(auto c : mesh.cells(owned)) {
    f(c).a = double(c->id()) * 1000.0;
    f(c).b = c->id();
    f(c).v[0] = double(c->id());
    f(c).v[1] = double(c->id()) + 1.0;
    f(c).v[2] = double(c->id()) + 2.0;
  } // for
} // initialize_field

flecsi_register_task(initialize_field, example, loc, single);

// This task prints the struct values.

void
print_field(mesh<ro> mesh, struct_field<ro> f) {
  for(auto c : mesh.cells(owned)) {
    std::cout << "cell id: " << c->id() << " has value: " << std::endl;
    std::cout << "\ta: " << f(c).a << std::endl;
    std::cout << "\tb: " << f(c).b << std::endl;
    std::cout << "\tv[0]: " << f(c).v[0] << std::endl;
    std::cout << "\tv[1]: " << f(c).v[1] << std::endl;
    std::cout << "\tv[2]: " << f(c).v[2] << std::endl;
  } // for
} // print_field

flecsi_register_task(print_field, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

  // The interface for retrieving a data handle now uses the
  // struct_type_t type.

  auto f = flecsi_get_handle(m, types, f, struct_type_t, dense, 0);

  flecsi_execute_task(initialize_field, example, single, m, f);
  flecsi_execute_task(print_field, example, single, m, f);

} // driver

} // namespace execution
} // namespace flecsi
