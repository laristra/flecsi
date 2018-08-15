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

#include <cstdlib>
#include <iostream>

#include<flecsi-tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

// Field registration is as usual (but specifying the 'sparse'
// storage class).

flecsi_register_field(mesh_t, example, field, double, sparse, 1, cells);

namespace example {

// This task takes a mesh and a sparse mutator and randomly populates
// field entries into the sparse field structure.

void initialize_sparse_field(mesh<ro> mesh, sparse_field_mutator f) {

  for(auto c: mesh.cells()) {
    const size_t random = (rand()/double{RAND_MAX}) * 5;

    for(size_t i{0}; i<random; ++i) {
      const size_t entry = (rand()/double{RAND_MAX}) * 5;

      // Note that the field operator () takes a cell index and
      // an entry. This provides logically dense access to sparsley
      // stored data.

      f(c, entry) = entry;

    } // for
  } // for
} // initialize_pressure

flecsi_register_task(initialize_sparse_field, example, loc, single);

// This task prints the non-zero entries of the sparse field.

void print_sparse_field(mesh<ro> mesh, sparse_field<ro> f) {
  for(auto c: mesh.cells()) {
    for(auto m: f.entries(c)) {
      std::cout << f(c,m) << " ";
    } // for
    std::cout << std::endl;
  } // for
} // print_pressure

flecsi_register_task(print_sparse_field, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  // Get a handle to the mesh
  
  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

  {
  // Get a mutator to modify the sparsity structure of the data.

  auto f = flecsi_get_mutator(m, example, field, double, sparse, 0, 5);

  flecsi_execute_task(initialize_sparse_field, example, single, m, f);
  } // scope

  {
  // Get a handle to modify only the values of the data.

  auto f = flecsi_get_handle(m, example, field, double, sparse, 0);

  flecsi_execute_task(print_sparse_field, example, single, m, f);
  } // scope

} // driver

} // namespace execution
} // namespace flecsi
