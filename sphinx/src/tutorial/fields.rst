Example 4: Fields
=================

In FleCSI, a field is a logical array that is defined on an index space.
The elements of the field are defined through a template argument, so
that, with some restrictions, a field element can be any P.O.D. or
user-defined type.

Fields must be registered with the FleCSI runtime. This is somewhat
analogous to defining a variable in C++, except that variables in FleCSI
must be associated with an index space, i.e., they must be registered
*against* a valid index space of a specific data client. Internally,
this is similar to what one would do if they had a class or struct type
(the data client) in C++ and wanted to add a data member (the field) to
it. FleCSI uses some metaprogramming techniques and the FleCSI data
model to define field data. Field registration must occur at file scope,
i.e., outside of any executable code sections, and must specify a
compile-time type and index space. The arguments to the field
registration interface are:

1. The data client type. This must be a valid FleCSI data client (These
   are defined by the specialization layer.) In this example, the data
   client is *mesh_t*.

2. The namespace of the field. This does not need to be an actual C++
   namespace. This parameter is used to avoid naming collisions. In this
   example, the namespace is *example*.

3. The field name. This is an arbitrary name created by the user to
   identify the field. It can be any legal C++ variable name. In this
   example, the field name is *field*.

4. The data type. As stated above, this can be a P.O.D. or user-defined
   type. The restrictions on this type are listed in the *NOTES* section
   below. In this example, the data type is *double*.

5. The storage class of the field. Storage class identifiers give the
   FleCSI runtime additional information about the logical structure of a
   field. They also define the interface that is available to the user for
   accessing the field data. Storage classes are covered in additional
   detail in a later example. In this example, the storage class is
   *dense*. The dense storage class is logically a contiguous array with
   one entry per index in the associated index space.

6. The number of versions to register. FleCSI field data support
   multiple versions of the data that are registered with a given
   namespace and name. This is useful for multi-step methods that
   require storage of multiple previous states, as well as other methods
   where it makes sense to store multiple copies of an array under a single
   name.

7. The index space. This must be a valid index space that is exposed by
   the data client type passed in argument 1. In this example, the index
   space is *cells*.

Users reference fields using a handle and an iterator of the
corresponding index space on which the field was registered. The current
interface is somewhat complicated and bears further discussion:

Logically, users *define* a task with arguments that may be a mixture of
by-value data, which are copied directly through the calling chain and
do not need to be registered with the FleCSI runtime, and data
accessors, which are types defined by the specialization or end-user
that correspond to registered field types. Notice that *define* is
emphasized in the previous sentence. This is to highlight the fact that
the definition of the task interface takes *accessors*, while the
*execution* of the task takes *handles*. Handles are opaque objects that
are returned to the end-user by calls to the handle interface. These
should generally use anonomous typeing, i.e., they should use the *auto*
keyword. The closest analogue in C++ to this paradigm is the passage of
variables *by-reference*, in which , a function or method is defined to
take a reference, but the user simply passes a variable instance. The
compiler automatically inserts logic to pass a reference to the variable
rather than copying it. Similarly, handles that are passed to the FleCSI
execution interface are automatically mapped to the associated accessor
type by the runtime, such that, from the point of view of the executing
task, the parameter is an accessor.

The handle interface takes the following arguments:

1. An instance of the data client against which the field was
   registered. Recall that registering a field is logically similar to
   adding a data member to a type. Passing a data client instance tells
   the runtime for which type instance the field should be returned.
   Because fields are stateful, there must be one field instance for
   each data client instance of the associated client type. Nominally,
   allocation of field data is lazy, such that a client instance for
   which a particular field is never accessed, never allocates the
   field. In this example, the data client instance is *m*.

2. The namespace of the field. In this example, the namespace is *example*.

3. The field name. In this example, the field name is *field*.

4. The data type. In this example, the data type is *double*.

5. The storage class of the field. In this example, the storage class is
   *dense*.

6. The version number of the field. This index is *zero-based*. In this
   example, the version is *0*.

NOTES:

* Field types may not reference arbitrary data, i.e., they cannot
  contain pointers to other data. Another way of saying this, is that
  all subtypes of a field must have a size that can be statically
  determined.

* The data client handle *m* and the field handle *f* are passed to the
  execution interface in the same order that they appear in the task
  definition.

The code for this example can be found in *fields.cc*:

.. code-block:: cpp

  #include <iostream>

  #include<flecsi-tutorial/specialization/mesh/mesh.h>
  #include<flecsi/data/data.h>
  #include<flecsi/execution/execution.h>

  using namespace flecsi;
  using namespace flecsi::tutorial;

  // This call registers a field called 'field' against a data client
  // with type 'mesh_t' on the index space 'cells'. The field is in the
  // namespace 'example', and is of type 'double'. The field has storage
  // class 'dense', and has '1' version.

  flecsi_register_field(mesh_t, example, field, double, dense, 1, cells);

  namespace example {

  // This task takes mesh and field accessors and initializes the
  // field with the id of the cell on which it is defined.

  void initialize_field(mesh<ro> mesh, field<rw> f) {
    for(auto c: mesh.cells(owned)) {
      f(c) = double(c->id());
    } // for
  } // initialize_field

  // Task registration is as usual...

  flecsi_register_task(initialize_field, example, loc, single);

  // This task prints the field values.

  void print_field(mesh<ro> mesh, field<ro> f) {
    for(auto c: mesh.cells(owned)) {
      std::cout << "cell id: " << c->id() << " has value " <<
        f(c) << std::endl;
    } // for
  } // print_field

  // Task registration is as usual...

  flecsi_register_task(print_field, example, loc, single);

  } // namespace example

  namespace flecsi {
  namespace execution {

  void driver(int argc, char ** argv) {

    // Get data handles to the client and field

    auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
    auto f = flecsi_get_handle(m, example, field, double, dense, 0);

    // Task execution is as usual...

    flecsi_execute_task(initialize_field, example, single, m, f);
    flecsi_execute_task(print_field, example, single, m, f);

  } // driver

  } // namespace execution
  } // namespace flecsi

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
