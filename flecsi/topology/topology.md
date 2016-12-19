<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Topology Types) -->

# FIXME: Working notes for structured mesh interface

Neighbor information can be specified with from_dim, to_dim, and
thru_dim where:

* from_dim: The topological dimension of the entity for which neighbors
  should be found.

* to_dim: The topological dimension of the neighbors.

* thru_dim: The intermediate dimension across which the neighbor must be
  found, e.g.:

  -------------
  | 6 | 7 | 8 |
  -------------
  | 3 | 4 | 5 |
  -------------
  | 0 | 1 | 2 |
  -------------

  The neighbors of cell 0 thru dimension 0 are: 1, 3, 4
  The neighbors of cell 0 thru dimension 1 are: 1, 3

The interface for the structured mesh should provide a connectivities
interface that uses this information, e.g.:

    template<size_t from_dim, size_t to_dim, size_t thru_dim>
    iterator_type (to be defined)
    connectivities(
      size_t id
    )
    {
    } // connectivities

# Topology Types

FleCSI provides data structures and algorithms for several mesh and tree
types that are all based on a common design pattern, which allows static
specialization of the underlying data structure with user-provided
*entity* type definitions. The user specified types are defined by a
*policy* that specializes the low-level FleCSI data structure. The
following sections describe the currently supported types.

## Mesh Topology

The low-level mesh interface is parameterized by a policy, which defines
various properties such as mesh dimension, and concrete entity classes
corresponding to each domain and topological dimension. The mesh policy
defines a series of tuples in order to declare its entity types for each
topological dimension and domain, and select connectivities between each
entity. FleCSI supports a specialized type of localized connectivity
called a *binding*, which connects entities from one domain to
another domain.

FleCSI separates mesh topology from geometry, and the mesh--from the
topology's perspective--is simply a connected graph. Vertex coordinates
and other application data are part of the *state model*. Our
connectivity computation algorithms are based on DOLFIN.
Once vertices and cells have been created, the remainder of the
connectivity data is computed automatically by the mesh topology through
the following three algorithms: *build*, *transpose*, and *intersect*,
e.g., *build* is used to compute edges using cell-to-vertex connectivity
and is also responsible for creating entity objects associated with
these edges. From a connectivity involving topological dimensions $D_1
\rightarrow D_2$, transpose creates connectivity $D_2 \rightarrow D_1$.
Intersect, given $D_1 \rightarrow D'$ and $D' \rightarrow D_2$, computes
$D_1 \rightarrow D_2$.  

The low-level mesh topology provides a set of iterable objects that a
mid-level specialization can make available to an application to allow,
at a high-level, iteration through connectivities using an intuitive
*ranged-based for* syntax, e.g., forall cells $c_i$, forall edges
$e_i$ of cell $c_i$. Entities can be stored in sets that also support
range-based for iterations and enable set operations such as union,
intersection, difference, and provide functional model capabilities with
*filter*, *apply*, *map*, *reduce*, etc. 

## N-Tree Topology

The tree topology, applying the philosophy of mesh topology, supports a
$D$-dimensional tree topology, e.g., for $D = 3$, an octree. Similar to
meshes, the tree topology is parameterized on a policy that defines its
branch and entity types where branches define a container for entities
at the leafs as well as refinement and coarsening control. A tree
geometry class is used that is specialized for dimension and handles
such things as distance or intersection in locality queries. Branch id's
are mapped from geometrical coordinates to an integer using a Morton id
scheme so that branches can be stored in a hashed/unordered map--and,
additionally, pointers to child branches are stored for efficient
recursive traversal. Branch hashing allows for efficient random access,
e.g., given arbitrary coordinates, find the parent branch at the current
deepest level of the tree, i.e., the child's insertion branch.
Refinement and coarsening are delegated to the policy, and when an
application requests a refinement, the policy has control over when and
whether the parent branch is finally refined. Entity and branch sets
support the same set operations and functional operations as mesh
topology. C++11-style callable objects are used to make methods on the
tree generic but efficient. Various methods allow branches and entities
to be visited recursively. When an entity changes position, we allow an
entity's position in the tree to be updated as efficiently as possible
without necessarily requiring a reinsertion.

## K-D Tree Topology

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
