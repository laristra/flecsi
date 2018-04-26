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

#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>
#include<flecsi/data/global_accessor.h>

using namespace flecsi;

// This call registers storage for a pointer address on each color of the
// runtime. The uintptr_t type is a standard C++ type specifically designed
// to hold a pointer value.

flecsi_register_color(ns, name, uintptr_t, 1);
flecsi_register_color(ns, name1, double *, 1);

namespace example {

//----------------------------------------------------------------------------//
// MPI Tasks
//----------------------------------------------------------------------------//

// Task to allocate data from within the MPI runtime.

void mpi_new(color_accessor<uintptr_t, rw> p) {
  p = reinterpret_cast<std::uintptr_t>(new double);
} // mpi_task

flecsi_register_task(mpi_new, example, mpi, index);

// Task to access the allocated data from within the MPI runtime.

void mpi_use(color_accessor<uintptr_t, rw> p, double v) {
 double * ptr = reinterpret_cast<double *>(&p);
  *ptr = v;
} // mpi_task

flecsi_register_task(mpi_use, example, mpi, index);

// Task to print the data from within the MPI runtime.

void mpi_print(color_accessor<uintptr_t, ro> p) {
  double * ptr = reinterpret_cast<double *>(&p);
  std::cout << "mpi ptr: " << *ptr << std::endl;
} // legion_task

flecsi_register_task(mpi_print, example, mpi, index);

//----------------------------------------------------------------------------//
// FLeCSI Tasks
//----------------------------------------------------------------------------//

// Task to print the data from within the FleCSI runtime.

void print(color_accessor<uintptr_t, ro> p) {
  double * ptr = reinterpret_cast<double *>(&p);
  std::cout << "ptr: " << *ptr << std::endl;
} // legion_task

flecsi_register_task(print, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  auto color_handle=flecsi_get_color(ns, name, uintptr_t, 0);
  auto color_handle=flecsi_get_color(ns, name1, double *, 0);

  flecsi_execute_task(mpi_new, example, index, color_handle);

  flecsi_execute_task(mpi_use, example, index, color_handle, 3.1415926);

  flecsi_execute_task(print, example, single, color_handle);
  flecsi_execute_task(mpi_print, example, index, color_handle);

  flecsi_execute_task(mpi_use, example, index, color_handle, 6.2831852);
  flecsi_execute_task(print, example, single, color_handle);
  flecsi_execute_task(mpi_print, example, index, color_handle);
} // driver

} // namespace execution
} // namespace flecsi
