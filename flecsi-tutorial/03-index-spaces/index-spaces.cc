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

/*----------------------------------------------------------------------------*

 *----------------------------------------------------------------------------*/

flecsi_register_data_client(mesh_t, clients, mesh);

namespace hydro {

void simple(mesh<ro> mesh) {

  for(auto v: mesh.vertices()) {
    v->print("Hello World! I'm a vertex!");
  } // for

  for(auto c: mesh.cells(owned)) {
    c->print("Hello World! I am a cell!");

    for(auto v: mesh.vertices(c)) {
      v->print("I'm a vertex!");
    } // for

  } // for

} // simple

// Task registration is as usual...

flecsi_register_task(simple, hydro, loc, single);

} // namespace hydro

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  // Get a data client handle as usual...

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

  // Task execution is as usual...

  flecsi_execute_task(simple, hydro, single, m);

} // driver

} // namespace execution
} // namespace flecsi

/* vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : */
