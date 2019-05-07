Example 3: Index Spaces
=======================

Index spaces are a fundamental concept of the FleCSI programming model.
In principle, an index space is simply an enumerated set. In practice,
index spaces provide an elegant tool for reasoning about work division
and parallelism. One of the primary capabilities provided by the FleCSI
programming system is the automatic generation of index space iterators
from specializations of the core FleCSI data structures.

In this example, we demonstrate using the index spaces defined on a
simple mesh to iterate over the entities in the mesh. This expands on
the previous tutorial example, which introduced the notion of a data
client. Shown here, perhaps more concretely, is that a data client is a
specialization type that exposes one or more index spaces. In this case,
the data client is a simple mesh that provides an interface to the index
spaces, *cells*, and *vertices*. As can be seen in the code example,
users can iterate over these index spaces using straightforward
range-based loop constructs. This is the preferred style for accessing
entities that are defined on a FleCSI data client.

The entity types themselves are defined as part of the specialization.
This allows different specialization developers to use nomenclature and
interfaces that best reflect the intended use of their projects. As an
example, the vertex and cell types defined with our simple mesh both
have *print* methods that print a user-defined string and the entity's
id. Although these particular types are relatively trivial, an actual
specialization type may expose interfaces that are arbitrarily complex.

The code for this example can be found in *index-spaces.cc*:

.. code-block:: cpp

  #include <iostream>

  #include<flecsi-tutorial/specialization/mesh/mesh.h>
  #include<flecsi/data/data.h>
  #include<flecsi/execution/execution.h>

  using namespace flecsi;
  using namespace flecsi::tutorial;

  flecsi_register_data_client(mesh_t, clients, mesh);

  namespace example {

  void simple(mesh<ro> mesh) {

    // Iterate over the vertices index space

    for(auto v: mesh.vertices()) {
      v->print("Hello World! I'm a vertex!");
    } // for

    // Iterate over the cells index space, and then over
    // the vertices index space.

    for(auto c: mesh.cells(owned)) {
      c->print("Hello World! I am a cell!");

      for(auto v: mesh.vertices(c)) {
        v->print("I'm a vertex!");
      } // for
    } // for

  } // simple

  // Task registration is as usual...

  flecsi_register_task(simple, example, loc, single);

  } // namespace example

  namespace flecsi {
  namespace execution {

  void driver(int argc, char ** argv) {

    // Get a data client handle as usual...

    auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

    // Task execution is as usual...

    flecsi_execute_task(simple, example, single, m);

  } // driver

  } // namespace execution
  } // namespace flecsi

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
