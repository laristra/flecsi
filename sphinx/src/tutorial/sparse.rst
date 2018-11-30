Example 6: Sparse Data
======================

The FleCSI sparse storage class allows the representation of sparse data
fields, i.e., fields on which zero, one or more data may be defined at
each index of the associated index space. The sparse storage class is
suitable for representing sparse matrices, sparse materials, or any
other logical data structure that utilizes a compressed storage
appraoch. *Note that although the current implementation does use a
compressed storage scheme, there is no guaruntee that this will be the
case.* The sparse storage class defines the interface to the registered
field data, not the storage mechanism.

Sparse storage differs from simple contiguous data in that, not only are
the data at each index mutable, but the sparsity pattern of the data is
also mutable. Modifying the sparsity pattern of the data is a more
expensive operation than simply modifying the values of existing sparse
data. To allow for more efficient access patterns, FleCSI splits
operations that mutate the sparsity pattern from those that only modify
the data (without changing the sparsity pattern). Access to these is
defined using separate types:

1. A mutator allows *both* modification of the sparsity structure and
   of the data values.

2. A handle only allows modification to the data values.

The data handle interface is the same as for *dense* data. For the
mutator, the user must specify an additional argument that declares how
many slots should be pre-allocated for adding new non-sparse entries:

.. code-block:: cpp

  //                                                                v
  auto f = flecsi_get_mutator(m, example, field, double, sparse, 0, 5);

This argument is only a hint to the runtime: Slots added within this
allocation will be more efficient than those that exceed the allocation.
However, both operations will be correct, i.e., if the user has
specified 5 additional slots, but adds 6 non-sparse entries, the program
will still operate correctly. (FleCSI uses an overflow buffer to manage
additional entries that exceed the slot allocation.)

NOTES:

* Tasks that do not need to mutate the sparsity structure should always
  use an *handle* rather than a *mutator* for efficiency.

* We are investigating design changes that will allow specialization
  developers to add new storage classes and storage class
  implementations.

.. code-block:: cpp

  #include <cstdlib>
  #include <iostream>

  #include<flecsi-tutorial/specialization/mesh/mesh.h>
  #include<flecsi/data/data.h>
  #include<flecsi/execution/execution.h>

  using namespace flecsi;
  using namespace flecsi::tutorial;

  // Field registration is as usual (but specifying the 'sparse'
  // storage class).

  flecsi_register_field(mesh_t, example, field, double, sparse, 1, cells);

  namespace example {

  // This task takes a mesh and a sparse mutator and randomly populates
  // field entries into the sparse field structure.

  void initialize_sparse_field(mesh<ro> mesh, sparse_field_mutator f) {

    for(auto c: mesh.cells()) {
      const size_t random = (rand()/double{RAND_MAX}) * 5;

      for(size_t i{0}; i<random; ++i) {
        const size_t entry = (rand()/double{RAND_MAX}) * 5;

        // Note that the field operator () takes a cell index and
        // an entry. This provides logically dense access to sparsley
        // stored data.

        f(c, entry) = entry;

      } // for
    } // for
  } // initialize_pressure

  flecsi_register_task(initialize_sparse_field, example, loc, single);

  // This task prints the non-zero entries of the sparse field.

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

    // Get a handle to the mesh
    
    auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

    {
    // Get a mutator to modify the sparsity structure of the data.

    auto f = flecsi_get_mutator(m, example, field, double, sparse, 0, 5);

    flecsi_execute_task(initialize_sparse_field, example, single, m, f);
    } // scope

    {
    // Get a handle to modify only the values of the data.

    auto f = flecsi_get_handle(m, example, field, double, sparse, 0);

    flecsi_execute_task(print_sparse_field, example, single, m, f);
    } // scope

  } // driver

  } // namespace execution
  } // namespace flecsi

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
