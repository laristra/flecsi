.. |br| raw:: html

   <br />

   NOTES:

   registration
   topology_registration: add fields for topology meta data

   invocation
   init_args: add region requirement
   prologue: updates data dependencies
   epilogue: set dirty (modified) bit

   enactment
   bind_accessors: binds data buffers to accessors
   unbind_accessors: unbinds data buffers to accessors


Topology Requirements
=====================

To develop a new topology type from an existing type, e.g., mesh,
octree, or kd-tree, the user should follow these steps:

* Split Interface & Storage

* Define a topology handle type

* Specialize topology registration

* Specialize storage class implementations

Adding New Topologies
=====================

1. **Topology Type**: Add a new subdirectory to the *flecsi/topology* directory named for
   the new topology type, e.g., *ntree*.
   
   This subdirectory should include:

   * interface.h: This file defines the topology interface, e.g.,

     .. code-block:: cpp

       namespace flecsi {
       namespace topology {

       template<typename POLICY_TYPE>
       struct ntree_topology_u : public ntree_topology_base_t {

         // interface ...

       }; // struct ntree_topology_u

       } // namespace flecsi
       } // namespace topology

   * types.h: This file defines types that are used by FleCSI, and by
     the new topology type. At a minimum, this file should define a base
     type from which the new topology type shall inherit, and a *coloring_t*
     type. The base class will be used to identify specializations of
     the new type in explicit specializations and template function
     overloads. The coloring type should include whatever interface and
     data members are required to form a distributed-memory
     representation of the new topology:

     .. code-block:: cpp

       struct ntree_topology_base_t {
         
       using coloring_t = ntree_topology_coloring_t;

         // interface ...

       }; // struct ntree_topology_u

     The base type should be named consistently with the new topology
     type name, and should follow FleCSI naming conventions. The base
     type must define the public *coloring_t* type.

2. **Topology Registration**: Define an partial specialization of the *topology_registration_u*
   type in *flecsi/data/common/topology_registration.h*. This type must
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
     struct  topology_instance_u<ntree_topology_u<POLICY_TYPE>> {

       using topology_reference_t =
         topology_reference_u<ntree_topology_u<POLICY_TYPE>>;

       static void set_coloring(topology_reference_t const & topology_reference,
         ntree_topology_u<POLICY_TYPE>::coloring_t const & colorint) {
       } // set_coloring

     }; // topology_instance_u<ntree_topology_u<POLICY_TYPE>>

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
