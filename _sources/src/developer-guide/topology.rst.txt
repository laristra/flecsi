.. |br| raw:: html

   <br />

Topology Design Idioms
======================

FleCSI uses three C++ idioms to implement topology types:
template specialization, template function overload, and tuple walkers.

* **Template Specialization** |br|
  Explicit or partial template specializations use C++ type inference to
  match a template type parameter to a specific implementation, thus
  allowing users to customize the behavior of a class or struct
  depending on the type of the parameter that is passed:

  .. code-block:: cpp

    // Unspecialized (default) behavior.
    template<typename TYPE>
    struct type {};

    template<>
    struct type<match_type_one_t> {

      static void method() {
        // Implementation for type one.
      } // method

    }; // struct type

    template<>
    struct type<match_type_two_t> {

      static void method() {
        // Implementation for type two.
      } // method

    }; // struct type

  The distinction between *explicit* and *partial* specialization is
  whether or not the specialized type is fully (explicit) or partially
  qualified, i.e., partially specialized types leave some template
  parameters unspecified.

* **Template Function Overload** |br|
  Template function overloads are similar to template specialization,
  but allow specialization of a method or function:

  .. code-block:: cpp

    struct enclosing_type_t {

      template<typename TYPE>
      void method(type_one<TYPE> & arg) {
        // Implementation for type one.
      } // method

      template<typename TYPE>
      void method(type_two<TYPE> & arg) {
        // Implementation for type two.
      } // method

    }; // struct enclosing_type_t

* **Tuple Walker** |br|
  The tuple walker idiom is really a calling technique used in
  conjunction with template function overload that applies a *visit*
  method, defined via the function template overload, to each
  *type* or *value* of a
  `std::tuple <https://en.cppreference.com/w/cpp/utility/tuple>`_. The
  distinction between tuple types and values is an important one, as it
  distinguishes between static (type information, available at compile
  time) and dynamic (variable information, available at runtime) state.

  An example of FleCSI's use of the tuple walker idiom, applied to
  dynamic tuple values, is the *task_prologue_t* type, used to add region
  requirements for the Legion implementation of *flecsi_execute_task*:

  .. code-block:: cpp

    struct task_prologue_t : public
    flecsi::utils::tuple_walker<task_prologue_t> {

      template<typename DATA_TYPE, size_t PRIVILEGES>
      void visit(index_topology::accessor<DATA_TYPE, PRIVILEGES> &
      accessor) {
        // Implmentation for type
        // index_topology::accessor<DATA_TYPE, PRIVILEGES>.
      } // visit

    }; // struct task_prologue_t

  You may notice that I lied to you before about there only being three
  idioms: Our tuple walker type also uses the CRTP idiom documented
  `here <http://laristra.github.io/flecsi/src/developer-guide/patterns/CRTP.html>`_.

Adding New Topologies
=====================

1. **Topology Type**: Add a new subdirectory to the *flecsi/topology*
   directory named for the new topology type, e.g., *ntree*.
   
   This subdirectory should include:

   * interface.h: This file defines the topology interface, e.g.,

     .. code-block:: cpp

       namespace flecsi {
       namespace topology {

       template<typename POLICY_TYPE>
       struct ntree_topology : public ntree_topology_base_t {

         // interface ...

       }; // struct ntree_topology

       } // namespace flecsi
       } // namespace topology

   * types.h: This file defines types that are used by FleCSI, and by
     the new topology type. At a minimum, this file should define a base
     type from which the new topology type shall inherit, and a
     *coloring_t* type. The base class will be used to identify
     specializations of the new type in explicit/partial specializations
     and template function overloads. The coloring type should include
     whatever interface and data members are required to form a
     distributed-memory representation of the new topology:

     .. code-block:: cpp

       struct ntree_topology_base_t {
         
       using coloring_t = ntree_topology_coloring_t;

         // interface ...

       }; // struct ntree_topology_base_t

     The base type should be named consistently with the new topology
     type name, and should follow FleCSI naming conventions. The base
     type must define the public *coloring_t* type.

2. **Topology Registration**: Define a partial specialization of the
   *topology_registration* type in
   *flecsi/data/topology_registration.h*. This type must
   implement a *register_fields* method that adds the fields required to
   represent the meta data associated with an instance of the new
   topology type.

3. **Topology Instance**: Define runtime-specific topology instance types in
   *data/runtime/topologies.h*, where *runtime* is implemented for each
   supported backend runtime type, e.g., Legion, MPI, and HPX
   (currently).

   The new type must define a *set_coloring* method that takes the
   *coloring_t* type defined in assocaited *types.h* file:

   .. code-block:: cpp

     template<typename POLICY_TYPE>
     struct  topology_instance<ntree_topology<POLICY_TYPE>> {

       using topology_reference_t =
         topology_reference<ntree_topology<POLICY_TYPE>>;

       static void set_coloring(topology_reference_t const & topology_reference,
         ntree_topology<POLICY_TYPE>::coloring_t const & colorint) {
       } // set_coloring

     }; // topology_instance<ntree_topology<POLICY_TYPE>>

4. **Initialize Arguments**: Define a template function
   overload of the *task_prologue_t* type in
   *flecsi/execution/.../task_prologue.h* that adds the region
   requirements for the given type instance (for Legion only),
   updates distributed-memory data dependencies, and
   sets a dirty (modified) bit for any fields or topologies that were
   accessed with write privileges (write-only, or read-write).

5. **Bind Accessors**: Define a template function overload of the
   *bind_accessors_t* type in
   *flecsi/execution/runtime/bind_accessors.h*, where
   *runtime* is implmented for each backend runtime. This function binds
   backend data buffers into the topology accesor instance. The accessor
   is defined as part of the topology type, and implements a
   *proxy* `pattern <https://en.wikipedia.org/wiki/Proxy_pattern>`_.

6. **Unbind Accessors**: Define a template function overload of the
   *unbind_accessors_t* type in
   *flecsi/execution/runtime/unbind_accessors.h*, where
   *runtime* is implmented for each backend runtime. This function unbinds
   backend data buffers, and does any cleanup operations that are
   necessary to complete task execution, e.g., committing changes to
   sparse or dynamic storage class fields.

Topology Initialization Workflow
================================

1. User defines specialization policy

2. User defines topology type with policy

3. Register meta data fields for specialized topology type

4. User adds fields to topology-defined index spaces

5. User gets topology instance

6. User generates coloring and calls set_coloring on instance

7. FleCSI creates index spaces and index partitions

8. FleCSI invokes task to initialize topology meta data

9. User invokes task to initialize field state

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
