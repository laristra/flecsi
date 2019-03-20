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

#include <flecsi-tutorial/specialization/mesh/mesh.h>
#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

// This call registers a field called 'field' against a data client
// with type 'mesh_t' on the index space 'cells'. The field is in the
// namespace 'example', and is of type 'double'. The field has storage
// class 'dense', and has '1' version.

flecsi_register_field(mesh_t, example, field, double, dense, 1, cells);

namespace example {

// This task takes mesh and field accessors and initializes the
// field with the id of the cell on which it is defined.

void
initialize_field(mesh<ro> mesh, field<rw> f) {
  for(auto c : mesh.cells(owned)) {
    f(c) = double(c->id());
  } // for
} // initialize_field

// Task registration is as usual...

flecsi_register_task(initialize_field, example, loc, single);

// This task prints the field values.

void
print_field(mesh<ro> mesh, field<ro> f) {
  for(auto c : mesh.cells(owned)) {
    clog(info) << "cell id: " << c->id() << " has value " << f(c) << std::endl;
  } // for
} // print_field

// Task registration is as usual...

flecsi_register_task(print_field, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {

  // Get data handles to the client and field

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto f = flecsi_get_handle(m, example, field, double, dense, 0);

  // Task execution is as usual...

  flecsi_execute_task(initialize_field, example, single, m, f);
  flecsi_execute_task(print_field, example, single, m, f);

} // driver

} // namespace execution
} // namespace flecsi
