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

#include <cstdlib>
#include <iostream>

#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>
#include<flecsi/data/global_accessor.h>

using namespace flecsi;

flecsi_register_color(ns, name, double, 1);

namespace example {

void mpi_new(color_accessor<double, rw> p) {
  std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(new double);
  p = ptr;
} // mpi_task

flecsi_register_task(mpi_new, example, mpi, index);

void mpi_use(color_accessor<double, rw> p, double v) {
 double * ptr = reinterpret_cast<double *>(&p);
  *ptr = v;
} // mpi_task

flecsi_register_task(mpi_use, example, mpi, index);

void print(color_accessor<double, ro> p) {
  double * ptr = reinterpret_cast<double *>(&p);
  std::cout << "ptr: " << *ptr << std::endl;
} // legion_task

flecsi_register_task(print, example, loc, single);

void mpi_print(color_accessor<double, ro> p) {
  double * ptr = reinterpret_cast<double *>(&p);
  std::cout << "mpi ptr: " << *ptr << std::endl;
} // legion_task

flecsi_register_task(mpi_print, example, mpi, index);

} // namespace example

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

   auto color_handle=flecsi_get_color( ns, name, double, 0);
  flecsi_execute_task(mpi_new, example, index, color_handle);
  flecsi_execute_task(mpi_use, example, index, color_handle, 1.0);
  flecsi_execute_task(print, example, single, color_handle);
  flecsi_execute_task(mpi_print, example, index, color_handle);
  flecsi_execute_task(mpi_use, example, index, color_handle, 2.0);
  flecsi_execute_task(print, example, single, color_handle);
} // driver

} // namespace execution
} // namespace flecsi
