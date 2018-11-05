# FleCSI Doxygen Mainpage

FleCSI is a compile-time configurable C++ framework designed to support
multi-physics application development. As such, FleCSI attempts to
provide a very general set of infrastructure design patterns that can be
specialized and extended to suit the needs of a broad variety of solver
and data requirements.  Current support includes multi-dimensional mesh
topology, mesh geometry, and mesh adjacency information, n-dimensional
hashed-tree data structures, graph partitioning interfaces, and
dependency closures (to identify data dependencies between
distributed-memory address spaces).

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
three-tiered approach: a low-level **core** library that is customized
by a mid-level **specialization** layer to create a **high-level
application interface**. This structure facilitates separation of
concerns, both between developer roles, and between the structural
components that make up a FleCSI-based application.

## FleCSI Code Structure

The FleCSI code base is divided into several namespaces:

* **flecsi::data**<br>
  FleCSI data model types and interfaces.

* **flecsi::execution**<br>
  FleCSI execution model types and interfaces.

* **flecsi::topology**<br>
  FleCSI graph, mesh, and tree topology data structures and interfaces.

* **flecsi::geometry**<br>
  FleCSI geometry types and interfaces.

* **flecsi::dmp**<br>
  FleCSI distributed-memory parallel types and interfaces.

* **flecsi::io**<br>
  FleCSI I/O types and interfaces.

* **flecsi::utils**<br>
  FleCSI utility types and functions.

Each of these is described in more detail in the developer guide.
Documentation is also provide in the corresponding Doxygen
[modules](modules.html).

## Naming Conventions

Because FleCSI makes extensive use of C++ templates, type names are
designed to indicate whether or not a type is fully qualified (A fully
qualified type is one for which all template parameters have been
specified.) Fully qualified types are of the form *type_name_t*, where
the underscore-t indicates that *type_name_t* is fully resolved. Types
that are not fully qualified are of the form *type_name_u*, where the
double-underscore indicates that the type still has unresolved
parameters. The following code illustrates this convention:
```
// Unqualified type.
template<
  typename T
>
struct type_name_u {};

// Fully-qualified type.
using type_name_t = type_name_u<double>;
```

## Design Patterns

FleCSI makes use of several formal design patterns. Many of these are
documented in the [Related Pages](pages.html) section of the documentation.


## More Documentation

This document is intended only as documentation for the FleCSI C++
interface. The FleCSI Developer and User Guides are available
[here](https://flecsi.lanl.gov).

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
