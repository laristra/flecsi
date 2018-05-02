# FleCSI: Tutorial - 05 Sparse Data
<!--
  The above header is required for Doxygen to correctly name the
  auto-generated page. It is ignored in the FleCSI guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(tutorial::sparse-data) -->

# Sparse Data

The FleCSI sparse storage class allows the representation of sparse data
fields, i.e., fields on which one or more data may be defined at each
index of the associated index space. The sparse storage class is
suitable for representing sparse matrices, sparse materials, or any
other logical data structure that utilizes a compressed storage
appraoch. *Note that although the current implementation does use a
compressed storage scheme, there is no guaruntee that this will be the
case.* The sparse storage class defines the interface to the registered
field data, not the storage mechanism.  We are investigating design
changes that will allow specialization developers to add new storage
classes.

```cpp
#include <cstdlib>
#include <iostream>

#include<flecsi-tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

flecsi_register_data_client(mesh_t, clients, mesh);
flecsi_register_field(mesh_t, example, field, double, sparse, 1, cells);

namespace example {

void initialize_sparse_field(mesh<ro> mesh, sparse_field_mutator f) {

  for(auto c: mesh.cells()) {
    const size_t random = (rand()/double{RAND_MAX}) * 5;

    for(size_t i{0}; i<random; ++i) {
      const size_t index = (rand()/double{RAND_MAX}) * 5;
      f(c,index) = index;
    } // for
  } // for
} // initialize_pressure

flecsi_register_task(initialize_sparse_field, example, loc, single);

void print_sparse_field(mesh<ro> mesh, sparse_field<ro> f) {
  for(auto c: mesh.cells()) {
    for(auto m: f.entries(c)) {
      std::cout << f(c,m) << " ";
    } // for
    std::cout << std::endl;
  } // for
} // print_pressure

flecsi_register_task(print_sparse_field, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

  {
  auto f = flecsi_get_mutator(m, example, field, double, sparse, 0, 5);

  flecsi_execute_task(initialize_sparse_field, example, single, m, f);
  } // scope

  {
  auto f = flecsi_get_handle(m, example, field, double, sparse, 0);

  flecsi_execute_task(print_sparse_field, example, single, m, f);
  } // scope

} // driver

} // namespace execution
} // namespace flecsi
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
