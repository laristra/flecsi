Example 7: Ragged Data
======================

The FleCSI ragged storage class is similar to the *sparse* storage class,
but without *column* indices, i.e., data are compacted into a compressed
storage layout, but no information is stored about the position of
a non-zero datum within the associated entry space. This structure is
useful for creating *views* of data that may vary in size across the
associated index space. For example, consider a mixed element mesh where
the state model used for each element depends on the element type.  The
associated state values for each element do not have a global index (as
would be the case with weights for a sparse discretization matrix), but
their number varies per cell. The ragged storage class enables just this
sort of state representation.

The ragged storage interface uses similar semantics to sparse storage
with a few differences:

1. Like the sparse storage class, ragged storage uses mutators to modify
   the structure of the *raggedness*. Mutators can also modify existing
   data values. The size of the ragged data at an index is changed by
   calling the *resize* method.

2. As stated above, ragged data do not have indexed entries, i.e., there
   is no implied position within the ragged array data for a given entry.
   The ragged array at each index is simply an array of the specified
   data type with a particular size.

.. code-block:: cpp

  #include <cstdlib>
  #include <iostream>

  #include<flecsi-tutorial/specialization/mesh/mesh.h>
  #include<flecsi/data/data.h>
  #include<flecsi/execution/execution.h>

  using namespace flecsi;
  using namespace flecsi::tutorial;

  // Field registration is as usual (but specifying the 'ragged'
  // storage class).

  flecsi_register_field(mesh_t, hydro, densities, double, ragged, 1, cells);

  namespace hydro {

  // This task takes a mesh and a ragged mutator and randomly populates
  // field entries into the ragged field structure.

  void initialize_materials(mesh<ro> mesh, ragged_field_mutator d) {

    for(auto c: mesh.cells(owned)) {
      const size_t random = (rand()/double{RAND_MAX}) * 5;

      d.resize(c, random);

      for(size_t i{0}; i<random; ++i) {
        d(c, i) = i;
      } // for
    } // for
  } // initialize_pressure

  flecsi_register_task(initialize_materials, hydro, loc, single);

  // This task prints the non-zero entries of the ragged field.

  void print_materials(mesh<ro> mesh, ragged_field<ro> d) {
    for(auto c: mesh.cells()) {
      for(auto m: d.entries(c)) {
        std::cout << d(c,m) << " ";
      } // for
      std::cout << std::endl;
    } // for
  } // print_pressure

  flecsi_register_task(print_materials, hydro, loc, single);

  } // namespace hydro

  namespace flecsi {
  namespace execution {

  void driver(int argc, char ** argv) {

    auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

    {
    // Get a mutator to modify the ragged structure of the data.

    auto d = flecsi_get_mutator(m, hydro, densities, double, ragged, 0, 5);

    flecsi_execute_task(initialize_materials, hydro, single, m, d);
    } // scope

    {
    // Get a handle to modify only the values of the data.

    auto d = flecsi_get_handle(m, hydro, densities, double, ragged, 0);

    flecsi_execute_task(print_materials, hydro, single, m, d);
    } // scope

  } // driver

  } // namespace execution
  } // namespace flecsi

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
