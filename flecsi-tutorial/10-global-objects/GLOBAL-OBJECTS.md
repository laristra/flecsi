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
be set during an appropriate phase of execution, and are available
during task execution. The types themselves may have any legal C++
interface, and they are not restricted by the general data model rules
of FleCSI. The FleCSI global object interface has three methods:
*flecsi_register_global_object*, *flecsi_set_global_object*, and
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

The set interface takes the following arguments:

1. The name of the global object instance.

2. The namespace of the object instance.

3. A pointer to the allocated object instance.

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

The code for this example can be found in *global-objects.cc*:

```cpp
#include <iostream>
#include <cstdlib>

#include<flecsi-tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

enum identifier_t : size_t {
  type_1,
  type_2
}; // enum identifier_t

struct data_t {
  identifier_t id;
}; // struct data_t

template<
size_t SHARED_PRIVILEGES>
using cell_data = dense_accessor<data_t, rw, SHARED_PRIVILEGES, ro>;

struct base_t {
  virtual ~base_t() {}

  virtual double compute(double x, double y) = 0;
}; // struct base_t

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

struct type_2_t : public base_t {

  double compute(double x, double y) override {
    return x*y;
  } // compute

}; // struct type_2_t

namespace example {

// Define a task to initialize the cell data

void update(mesh<ro> m, cell_data<rw> cd) {
  for(auto c: m.cells(owned)) {
    const size_t flip = double(rand())/RAND_MAX + 0.5;

    if(flip) {
      cd(c).id = type_1;
    }
    else {
      cd(c).id = type_2;
    } // if
  } // for
} // update

flecsi_register_task(update, example, loc, single);

void print(mesh<ro> m, cell_data<ro> cd) {
  for(auto c: m.cells(owned)) {
    auto derived = flecsi_get_global_object(cd(c).id, derived, base_t);

    std::cout << "compute: " << derived->compute(1.0, 1.0) << std::endl;
  } // for
} // print

flecsi_register_task(print, example, loc, single);

} // namespace example

flecsi_register_data_client(mesh_t, clients, mesh);
flecsi_register_field(mesh_t, example, cell_data,
  data_t, dense, 1, cells);

flecsi_register_global_object(type_1, derived, base_t);
flecsi_register_global_object(type_2, derived, base_t);

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  // This should move into the specialization
  flecsi_set_global_object(type_1, derived, base_t, new type_1_t(1.0, 2.0));
  flecsi_set_global_object(type_2, derived, base_t, new type_1_t(3.0, 4.0));

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto cd = flecsi_get_handle(m, example, cell_data, data_t, dense, 0);

  flecsi_execute_task(update, example, single, m, cd);
  flecsi_execute_task(print, example, single, m, cd);

} // driver

} // namespace execution
} // namespace flecsi
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
