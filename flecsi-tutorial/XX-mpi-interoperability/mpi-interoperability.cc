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
  Documentation for this example can be found in MPI-INTEROPERABILITY.md.
 *----------------------------------------------------------------------------*/

flecsi_register_color(ns, name, uintptr_t, 1);

namespace example {

void mpi_new(color_accessor<uintptr_t, rw> p) {
  p = reinterpret_cast<uintptr_t>(new double);
} // mpi_task

flecsi_register_task(mpi_new, example, mpi, single);

void mpi_use(color_accessor<uintptr_t, rw> p) {
  double * ptr = reinterpret_cast<double *>(p);
  *ptr = 1.0;
} // mpi_task

flecsi_register_task(mpi_use, example, mpi, single);

void print(color_accessor<uintptr_t, ro> p) {
  double * ptr = reinterpret_cast<double *>(p);
  std::cout << "ptr: " << *ptr << std::endl;
} // mpi_task

flecsi_register_task(print, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

} // driver

} // namespace execution
} // namespace flecsi
