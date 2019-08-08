# FleCSI: Tutorial - 10 Global Objects
<!--
  The above header is required for Doxygen to correctly name the
  auto-generated page. It is ignored in the FleCSI guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(tutorial::dense-data) -->

# Global Objects

<span style="color:red">
*Advanced Topic: Intended for Specialization Developers*
</span>

The data model enforced by the FleCSI runtime makes it illegal to
implement traditional C++ object models that employ dynamic
polymorphism. In general, you are not allowed to register data types
with the runtime that use virtual tables, i.e., the FleCSI data model
does not allow virtual or pure virtual types. Experimentation with
various application projects has shown that virtual inheritance is a
useful design pattern for certain dynamic data needs. The FleCSI global
object interface provides a mechanism to allow some of these patterns,
provided that they follow particular initialization and usage rules.

*NOTE: The restriction on dynamic polymorphism arises from the
possibility that data that are registered with the FleCSI runtime may be
mapped or moved to multiple memory spaces during the execution of a
program. These operations would invalidate the virtual method table
(VMT) used to support dynamic dispatch (runtime binding), making the
interface invalid.*

One pattern of dynamic polymorphism that is safe involves object
instances that are created during an initialization phase on each
logical color of a distributed-memory program. In this case, each unit
of execution (color) must execute the same steps. This allows us to
reason about what execution path has taken place, regardless of how the
processes have been mapped to node resources. Using this reasoning,
FleCSI can support a global object interface that allows users to
register dynamically polymorphic types with the runtime. These can then
be initialized during an appropriate phase of execution, and are
available during task execution. The types themselves may have any legal
C++ interface, and they are not restricted by the general data model
rules of FleCSI. The FleCSI global object interface has three methods:
*flecsi_register_global_object*, *flecsi_initialize_global_object*, and
*flecsi_get_global_object*.

The registration interface takes the following arguments:

1. The name of the global object instance (user-defined). In this
   example, the names are *type_1* and *type_2*. The name is used as an
   identifier for subsequent calls to the interface. In this example, the
   identifier is actually an enumerated type value. This is useful if
   the user wants to switch over various derived types using only an
   integer id.

2. The namespace of the object instance (user-defined). This is used to
   avoid naming collisions. In this example, the namespace is *derived*.

3. The base class type of the global object instance. This must be a
   valid C++ type. In this example, the base class is *base_t*.

The initialization interface takes the following arguments:

1. The name of the global object instance.

2. The namespace of the object instance.

3. The type with which the object should be initialized. In general,
   this is a derived type. Initialization will instantiate a new object of
   this type.

4. A variadic argument list that will be passed to the constructor of
   the type argument.

The get interface takes the following arguments:

1. The name of the global object instance.

2. The namespace of the object instance.

3. The base class type of the global object instance. This must be a
   valid C++ type. In this example, the base class is *base_t*.

NOTES:

* It is currently only possible to set global objects in a
  specialization initialization control point. This is intended to avoid
  possible race conditions. This restriction may be relaxed in future
  versions.

* Global object instances are managed by the runtime, e.g., the
  destructor will be called for each global object that is registered.

The code for this example can be found in *global-objects.cc*:

```cpp
#include <iostream>
#include <cstdlib>

#include<flecsi/tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

// Create an identifier type. This will allow us to switch between
// object instances using an integer id.

enum identifier_t : size_t {
  type_1,
  type_2
}; // enum identifier_t

// Create a data type to store the integer id.

struct data_t {
  identifier_t id;
}; // struct data_t

// Define an accessor type to use as the task argument.

template<
size_t SHARED_PRIVILEGES>
using cell_data = dense_accessor<data_t, rw, SHARED_PRIVILEGES, ro>;

// This is a simple base type with one pure virtual method that we will
// use to demonstrate the global object interface.

struct base_t {
  virtual ~base_t() { std::cout << "delete called" << std::endl; }

  virtual double compute(double x, double y) = 0;

}; // struct base_t

// A derived type with a non-trivial constructor.

struct type_1_t : public base_t {

  type_1_t(double w0, double w1)
    : w0_(w0), w1_(w1) {}

  double compute(double x, double y) override {
    return w0_*x + w1_*y;
  } // compute

private:

  double w0_;
  double w1_;

}; // struct type_1_t

// A derived type with a trivial constructor.

struct type_2_t : public base_t {

  double compute(double x, double y) override {
    return x*y;
  } // compute

}; // struct type_2_t

namespace example {

// Define a task to initialize the cell data. This will randomly pick
// one of the integer ids for each cell.

void update(mesh<ro> m, cell_data<rw> cd) {
  for(auto c: m.cells(owned)) {
    const size_t flip = double(rand())/RAND_MAX + 0.5;
    cd(c).id = flip ? type_1 : type_2;
  } // for
} // update

flecsi_register_task(update, example, loc, single);

// Print the results of executing the "compute" method.

void print(mesh<ro> m, cell_data<ro> cd) {
  for(auto c: m.cells(owned)) {

    // This call gets the global object associated with the id we
    // randomly set in the update task.

    auto derived = flecsi_get_global_object(cd(c).id, derived, base_t);

    std::cout << "compute: " << derived->compute(5.0, 1.0) << std::endl;
  } // for
} // print

flecsi_register_task(print, example, loc, single);

} // namespace example

// Normal registration of the data client and cell data.

flecsi_register_data_client(mesh_t, clients, mesh);
flecsi_register_field(mesh_t, example, cell_data,
  data_t, dense, 1, cells);

// Register the derived object instances that we will initialize and
// use in the example.

flecsi_register_global_object(type_1, derived, base_t);
flecsi_register_global_object(type_2, derived, base_t);

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

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

  flecsi_execute_task(update, example, single, m, cd);
  flecsi_execute_task(print, example, single, m, cd);

} // driver

} // namespace execution
} // namespace flecsi
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
