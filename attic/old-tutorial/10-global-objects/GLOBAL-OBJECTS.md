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
useful design pattern for certain dynamic data needs.
Such types may be used for variables of automatic or static duration.

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
processes have been mapped to node resources.
The actual initialization may be deferred until an appropriate phase of
execution (e.g., a specialization initialization control point) by using
`std::optional` or (to support polymorphism in a single variable)
`std::unique_ptr`.

The code for this example can be found in *global-objects.cc*:

```cpp
#include <cstdlib>
#include <iostream>
#include <optional>

#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>
#include <flecsi/tutorial/specialization/mesh/mesh.h>

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
using cell_data = dense_accessor<data_t, rw, SHARED_PRIVILEGES, ro>;

// This is a simple base type with one pure virtual method that we will
// use to demonstrate global object usage.

struct base_t {
  virtual ~base_t() {} // good practice, but not required with std::optional

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
std::optional<type_1_t> t1;
std::optional<type_2_t> t2;

// Define a task to initialize the cell data. This will randomly pick
// one of the integer ids for each cell.

void
update(mesh<ro> m, cell_data<rw> cd) {
  for(auto c : m.cells(owned)) {
    const size_t flip = double(rand()) / RAND_MAX + 0.5;
    cd(c).id = flip ? type_1 : type_2;
  } // for
} // update

flecsi_register_task(update, example, loc, single);

// Print the results of executing the "compute" method.

void
print(mesh<ro> m, cell_data<ro> cd) {
  for(auto c : m.cells(owned)) {

    // This call gets the global object associated with the id we
    // randomly set in the update task.

    auto & derived = cd(c).id == type_1 ? *t1 : *t2;

    std::cout << "compute: " << derived.compute(5.0, 1.0) << std::endl;
  } // for
} // print

flecsi_register_task(print, example, loc, single);

} // namespace example

// Normal registration of the data client and cell data.

flecsi_register_field(mesh_t, example, cell_data, data_t, dense, 1, cells);

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {

  // Initialization of the object instances. In a real code, this would
  // need to occur in the specialization initialization control point.

  example::t1.emplace(1, 2);
  example::t2.emplace();

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
