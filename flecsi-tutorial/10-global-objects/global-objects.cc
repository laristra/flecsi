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

  Global Object Example
  =====================

  The functional programming model enforced by the FleCSI runtime makes
  it difficult to implement traditional C++ object models that employ
  dynamic polymorphism. In general, you are not allowed to register data
  with the runtime that use virtual tables, i.e., the FleCSI data model
  does not allow virtual or pure virtual types. Experimentation with
  various application projects has shown that virtual inheritance is a
  useful design pattern for certain dynamic data needs.

 *----------------------------------------------------------------------------*/

#include <iostream>
#include <cstdlib>

#include<flecsi-tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

enum material_t : size_t {
  m1,
  m2
}; // enum material_t

struct cell_data_t {
  material_t m;
}; // struct cell_data_t

template<
size_t SHARED_PRIVILEGES>
using cell_data = dense_accessor<cell_data_t, rw, SHARED_PRIVILEGES, ro>;

struct base_t {
  virtual ~base_t() {}

  virtual double pressure(double r, double e) = 0;
}; // struct base_t

struct ideal_t : public base_t {

  ideal_t(double K, double gamma)
    : K_(K), gamma_(gamma) {}

  double pressure(double r, double e) override {
    return K_*r*(gamma_-1.0)*e;
  } // pressure

private:

  double K_;
  double gamma_;

}; // struct ideal_t

namespace eos_example {

// Define a task to initialize the cell data

void update(mesh<ro> m, cell_data<rw> cd) {
  for(auto c: m.cells(owned)) {
    const size_t flip = double(rand())/RAND_MAX + 0.5;

    if(flip) {
      cd(c).m = m1;
    }
    else {
      cd(c).m = m2;
    } // if
  } // for
} // update

flecsi_register_task(update, eos_example, loc, single);

void print(mesh<ro> m, cell_data<ro> cd) {
  for(auto c: m.cells(owned)) {
    auto eos = flecsi_get_global_object(cd(c).m, eos, base_t);

    std::cout << "pressure: " << eos->pressure(1.0, 1.0) << std::endl;
  } // for
} // print

flecsi_register_task(print, eos_example, loc, single);

} // namespace eos_example

flecsi_register_data_client(mesh_t, clients, mesh);
flecsi_register_field(mesh_t, eos_example, cell_data,
  cell_data_t, dense, 1, cells);

flecsi_register_global_object(m1, eos, base_t);
flecsi_register_global_object(m2, eos, base_t);

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  // This should move into the specialization
  flecsi_set_global_object(m1, eos, base_t, new ideal_t(1.0, 7.0/2.0));
  flecsi_set_global_object(m2, eos, base_t, new ideal_t(1.0, 1.66666));

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto cd = flecsi_get_handle(m, eos_example, cell_data, cell_data_t,
    dense, 0);

  flecsi_execute_task(update, eos_example, single, m, cd);
  flecsi_execute_task(print, eos_example, single, m, cd);

} // driver

} // namespace execution
} // namespace flecsi

/* vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : */
