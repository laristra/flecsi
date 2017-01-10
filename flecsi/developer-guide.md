<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Introduction) -->

\pagebreak

![](medium-flecsi.png "FleCSI Logo")\ 

\vspace{-1cm}

# Introduction

FleCSI is a compile-time configurable C++ framework designed to support
multi-physics application development. As such, FleCSI attempts to
provide a very general set of infrastructure design patterns that can be
specialized and extended to suit the needs of a broad variety of solver
and data requirements.  Current support includes multi-dimensional mesh
topology, mesh geometry, and mesh adjacency information, n-dimensional
hashed-tree data structures, graph partitioning interfaces, and
dependency closures, e.g., to identify data dependencies between
distributed-memory address spaces.

FleCSI also introduces a functional programming model with control,
execution, and data abstractions that are consistent state-of-the-art
task-based runtimes such as Legion and Charm++. The FleCSI abstraction
layer provides the developer with insulation from the underlying
runtime, while allowing support for multiple runtime systems, including
conventional models like asynchronous MPI.  The intent is to give
developers a concrete set of user-friendly programming tools that can be
used now, while allowing flexibility in choosing runtime implementations
and optimizations that can be applied to architectures and runtimes that
arise in the future.

FleCSI uses static polymorphism, template meta-programming techniques,
and other modern C++ features to achieve high runtime performance,
customizability, and to enable DSL-like features in our programming
model. In both the mesh and tree topology types, FleCSI adopts a
three-tiered approach: a low-level substrate that is specialized by a
mid-level layer to create high-level application interfaces that hide
the complexity of the underlying templated classes. This structure
facilitates separation of concerns, both between developer roles, and
between the structural components that make up a FleCSI-based
application. As an example, for a mesh of dimension $D_m$, the low-level
interface provides generic compile-time configurable components which
deal with *entities* of varying topological dimension $D_m$ (cell),
$D_m-1$, (face/edge), etc. Each of these entities resides in a *domain*
$M$ or sub-mesh. Entities are connected to each other by a
*connectivity* using a compressed id/offset representation for efficient
space utilization and fast traversal.

--------------------------------------------------------------------------------

<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Code Structure) -->

# Name Spaces

FleCSI uses several different namespaces:

* **data**  
  Data model types.

* **execution**  
  Execution model types.

* **io**  
  I/O types.

* **topology**  
  Topology types.

* **utils**  
  Utilities.

--------------------------------------------------------------------------------

<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Code Structure) -->

# Unit Tests

Unit tests should only be used to test specific components of FleCSI,
e.g., types or algorithms. **They shall not be used to develop new
applications!**. Sometimes, it *is* necessary to write a unit test that
depends on the FleCSI library itself. Developers should try to avoid
this dependence! However, when it is necessary, it is likely that such
a unit test will have unresolved symbols from the various runtime
libraries to which the FleCSI library links. To resolve these, the
developer should add ${CINCH_RUNTIME_LIBRARIES} to the LIBRARIES
argument to the cinch_add_unit() function:
```cmake
cinch_add_unit(mytest,
  SOURCES
    test/mytest.cc
  LIBRARIES
    flecsi ${CINCH_RUNTIME_LIBRARIES}
)
```

In addition to ${CINCH_RUNTIME_LIBRARIES}, ${CINCH_RUNTIME_INCLUDES},
and ${CINCH_RUNTIME_FLAGS} are also included.

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
