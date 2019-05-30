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

// This line is essentially just declaring a variable of type "mesh_t"
// named "example_mesh" in the namespace "clients".

flecsi_register_data_client(mesh_t, clients, example_mesh);

namespace hydro {

void
simple_task(mesh<rw> m) {

  // Our specialization mesh interface provides a "print" method. The
  // mechanism that allows a mesh handle to be passed to a task inherits
  // the interface of the data handle type, i.e., all of the methods of
  // the type are available to the user.

  m.print("Hello World! I am a mesh!");

} // simple_task

// Task registration is as usual...

flecsi_register_task(simple_task, hydro, loc, index);

} // namespace hydro

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {

  // Here, we ask the runtime for a handle to the data client variable
  // that we defined above. This handle is an opaque reference to the
  // data client variable that cannot be accessed outside of a task.
  // During task execution, this handle will be mapped into the task's
  // memory space, at which point it will be accessible by the user.

  auto m = flecsi_get_client_handle(mesh_t, clients, example_mesh);

  // Task execution is as usual...

  flecsi_execute_task(simple_task, hydro, index, m);

} // driver

} // namespace execution
} // namespace flecsi
