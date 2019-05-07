Example 5: Dense Data
=====================

The FleCSI data model provides several different storage types. A
storage type is a formal term that implies a particular logical layout
to the data of types registered under it. The *logical* layout of the
data provides the user with an interface that is consistent with a
particular view of the data. In this example, we focus on the *dense*
storage type.

Logically, dense data may be represented as a contiguous array, with
indexed access to its elements. The dense storage type interface is
similar to a C array or std::vector, with element access provided
through the *()* operator, i.e., if var has been registered as a dense
data type, it may be accessed like: *var(0)*, *var(1)*, etc.

In the *fields* example (04-fields), we were actually using the dense
storage type to represent an array of double defined on the cells of our
specialization mesh. This example is an extension that demonstrates
using user-defined types as dense field data.  

This example uses a user-defined struct as the dense data type. The
*struct_type_t* is defined in the *types.h* file:

.. code-block:: cpp

  #pragma once

  #include <flecsi/data/dense_accessor.h>

  namespace types {

  using namespace flecsi;

  // This is the definition of the struct_type_t type.

  struct struct_type_t {
    double a;
    size_t b;
    double v[3];
  }; // struct_type_t

  // Here, we define an accessor type to use in the task signatures in our
  // example. Notice that the base type "dense_accessor" takes four
  // parameters. The first parameter is the type that has been registered on
  // the associated index space. The other three parameters specify the
  // privileges with which the corresponding data will be accessed.

  template<
    size_t SHARED_PRIVILEGES>
  using struct_field = dense_accessor<struct_type_t, rw, SHARED_PRIVILEGES, ro>;

  } // namespace types

Aside from using a struct type, this example of registering data is
identical to registering a fundamental type, e.g., double or size_t.

The code for this example can be found in *dense.cc*:

.. code-block:: cpp

  #include <iostream>

  #include<flecsi-tutorial/specialization/mesh/mesh.h>
  #include<flecsi/data/data.h>
  #include<flecsi/execution/execution.h>

  #include "types.h"

  using namespace flecsi;
  using namespace flecsi::tutorial;
  using namespace types;

  flecsi_register_data_client(mesh_t, clients, mesh);
  flecsi_register_field(mesh_t, types, f, struct_type_t, dense, 1, cells);

  namespace example {

  // This task initializes the field of struct_type_t data. Notice that
  // nothing has changed about the iteration logic over the mesh. The only
  // difference is that now the dereferenced values are struct instances.

  void initialize_field(mesh<ro> mesh, struct_field<rw> f) {
    for(auto c: mesh.cells(owned)) {
      f(c).a = double(c->id())*1000.0;
      f(c).b = c->id();
      f(c).v[0] = double(c->id());
      f(c).v[1] = double(c->id())+1.0;
      f(c).v[2] = double(c->id())+2.0;
    } // for
  } // initialize_field

  flecsi_register_task(initialize_field, example, loc, single);

  // This task prints the struct values.

  void print_field(mesh<ro> mesh, struct_field<ro> f) {
    for(auto c: mesh.cells(owned)) {
      std::cout << "cell id: " << c->id() << " has value: " << std::endl;
      std::cout << "\ta: " << f(c).a << std::endl;
      std::cout << "\tb: " << f(c).b << std::endl;
      std::cout << "\tv[0]: " << f(c).v[0] << std::endl;
      std::cout << "\tv[1]: " << f(c).v[1] << std::endl;
      std::cout << "\tv[2]: " << f(c).v[2] << std::endl;
    } // for
  } // print_field

  flecsi_register_task(print_field, example, loc, single);

  } // namespace example

  namespace flecsi {
  namespace execution {

  void driver(int argc, char ** argv) {

    auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

    // The interface for retrieving a data handle now uses the
    // struct_type_t type.

    auto f = flecsi_get_handle(m, types, f, struct_type_t, dense, 0);

    flecsi_execute_task(initialize_field, example, single, m, f);
    flecsi_execute_task(print_field, example, single, m, f);

  } // driver

  } // namespace execution
  } // namespace flecsi

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
