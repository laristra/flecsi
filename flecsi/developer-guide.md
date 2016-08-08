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

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
