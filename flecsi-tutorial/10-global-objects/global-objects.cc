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
  Documentation for this example can be found in GLOBAL-OBJECTS.md.

  Note: This example covers advanced topics and is primarily intended
  for specialization developers.
 *----------------------------------------------------------------------------*/

#include <cstdlib>
#include <iostream>

#include <flecsi-tutorial/specialization/mesh/mesh.h>
#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

// Create an identifier type. This will allow us to switch between
// object instances using an integer id.

enum identifier_t : size_t { type_1, type_2 }; // enum identifier_t

// Create a data type to store the integer id.

struct data_t {
  identifier_t id;
}; // struct data_t

// Define an accessor type to use as the task argument.

template<size_t SHARED_PRIVILEGES>
using cell_data = dense_accessor<data_t, rw, SHARED_PRIVILEGES, rw>;

// This is a simple base type with one pure virtual method that we will
// use to demonstrate the global object interface.

struct base_t {
  virtual ~base_t() {}

  virtual double compute(double x, double y) = 0;

}; // struct base_t

// A derived type with a non-trivial constructor.

struct type_1_t : public base_t {

  type_1_t(double w0, double w1) : w0_(w0), w1_(w1) {}

  double compute(double x, double y) override {
    return w0_ * x + w1_ * y;
  } // compute

private:
  double w0_;
  double w1_;

}; // struct type_1_t

// A derived type with a trivial constructor.

struct type_2_t : public base_t {

  double compute(double x, double y) override {
    return x * y;
  } // compute

}; // struct type_2_t

namespace example {

// Define a task to initialize the cell data. This will randomly pick
// one of the integer ids for each cell.

void
update(mesh<ro> m, cell_data<rw> cd) {
  for(auto c : m.cells(owned)) {
    const size_t flip = double(rand()) / RAND_MAX + 0.5;
    cd(c).id = flip ? type_1 : type_2;
  } // for
} // update

flecsi_register_task(update, example, loc, index);

// Print the results of executing the "compute" method.

void
print(mesh<ro> m, cell_data<ro> cd) {
  for(auto c : m.cells(owned)) {

    // This call gets the global object associated with the id we
    // randomly set in the update task.

    auto derived = flecsi_get_global_object(cd(c).id, derived, base_t);

    std::cout << "compute: " << derived->compute(5.0, 1.0) << std::endl;
  } // for
} // print

flecsi_register_task(print, example, loc, index);

} // namespace example

// Normal registration of the data client and cell data.

flecsi_register_field(mesh_t, example, cell_data, data_t, dense, 1, cells);

// Register the derived object instances that we will initialize and
// use in the example.

flecsi_register_global_object(type_1, derived, base_t);
flecsi_register_global_object(type_2, derived, base_t);

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {

  // Initialization of the object instances. In a real code, this would
  // need to occur in the specialization initialization control point.
  //
  // Notice that the interface call accepts a variadic argument list
  // that is passed to the constructor of the particular type.

  flecsi_initialize_global_object(type_1, derived, type_1_t, 1.0, 2.0);
  flecsi_initialize_global_object(type_2, derived, type_2_t);

  // Get client and data handles as usual.

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto cd = flecsi_get_handle(m, example, cell_data, data_t, dense, 0);

  // Execute the tasks.

  flecsi_execute_task(update, example, index, m, cd);
  flecsi_execute_task(print, example, index, m, cd);

} // driver

} // namespace execution
} // namespace flecsi
