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

#include <flecsi/data/data.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/tutorial/specialization/mesh/mesh.h>

using namespace flecsi;
using namespace flecsi::tutorial;

// This field will be accessed by both the FleCSI and MPI runtimes.

flecsi_register_field(mesh_t, example, field, double, dense, 1, cells);

// This call registers storage for a pointer address on each color of the
// runtime. These data will be used to reference allocations that are
// created within an MPI task.

flecsi_register_color(ns, name, double *, 1);

namespace example {

//----------------------------------------------------------------------------//
// MPI Tasks
//----------------------------------------------------------------------------//

// This task is executed in the FleCSI runtime.

void
initialize_field(mesh<ro> mesh, field<rw> f) {
  for(auto c : mesh.cells(owned)) {
    f(c) = double(c->id());
  } // for
} // initialize_field

flecsi_register_task(initialize_field, example, loc, single);

// This task is executed in the MPI runtime.

void
access_field(mesh<ro> mesh, field<ro> f) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(auto c : mesh.cells(owned)) {
    clog(info) << "cell id: " << c->id() << std::endl;
    // std::cout << "rank: " << rank << " cell id: " << c->id() << std::endl;
  } // for
} // mpi_task

flecsi_register_task(access_field, example, mpi, index);

// Task to allocate data from within the MPI runtime.

void
mpi_new(color_accessor<double *, rw> p) {
  p = new double;

  clog(info) << "test" << std::endl;
  std::cout << "Pointer address at allocation: " << p << std::endl;
} // mpi_task

flecsi_register_task(mpi_new, example, mpi, index);

// Task to access the allocated data from within the MPI runtime.

void
mpi_use(color_accessor<double *, rw> p, double v) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  *p = (rank + 1.0) * v;
} // mpi_task

flecsi_register_task(mpi_use, example, mpi, index);

// Task to print the data from within the MPI runtime.

void
mpi_print(color_accessor<double *, ro> p) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::cout << "Rank " << rank << " with p=" << *p << std::endl;
} // legion_task

flecsi_register_task(mpi_print, example, mpi, index);

//----------------------------------------------------------------------------//
// FLeCSI Tasks
//----------------------------------------------------------------------------//

// Task to print the data from within the FleCSI runtime.

void
print(color_accessor<double *, ro> p) {
  std::cout << "Pointer address stored in FleCSI data model: " << p
            << std::endl;
} // legion_task

flecsi_register_task(print, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {

  // Initialize the dense field data through the FleCSI runtime.
  {
    auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
    auto f = flecsi_get_handle(m, example, field, double, dense, 0);

    flecsi_execute_task(initialize_field, example, single, m, f);
  } // scope

  // Access the field data through the MPI runtime.
  {
    auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
    auto f = flecsi_get_handle(m, example, field, double, dense, 0);

    flecsi_execute_task(access_field, example, index, m, f);
  } // scope

#if 0
  auto color_handle=flecsi_get_color(ns, name, double *, 0);

  flecsi_execute_task(mpi_new, example, index, color_handle);

  flecsi_execute_task(mpi_use, example, index, color_handle, 3.1415926);

  flecsi_execute_task(mpi_print, example, index, color_handle);
  flecsi_execute_task(print, example, single, color_handle);
#endif

  //  flecsi_execute_task(mpi_use, example, index, color_handle, 6.2831852);
  //  flecsi_execute_task(print, example, single, color_handle);
  //  flecsi_execute_task(mpi_print, example, index, color_handle);
} // driver

} // namespace execution
} // namespace flecsi
