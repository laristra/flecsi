Introduction
============

FleCSI is a compile-time configurable framework designed to support
multi-physics application development. As such, FleCSI provides a very
general set of infrastructure design patterns that can be specialized
and extended to suit the needs of a broad variety of solver and data
requirements. FleCSI currently supports multi-dimensional mesh topology,
geometry, and adjacency information, as well as n-dimensional
hashed-tree data structures, graph partitioning interfaces, and
dependency closures.

FleCSI introduces a functional programming model with control,
execution, and data abstractions that are consistent both with MPI and
with state-of-the-art, task-based runtimes such as Legion and Charm++.
The abstraction layer insulates developers from the underlying runtime,
while allowing support for multiple runtime systems including
conventional models like asynchronous MPI.

The intent is to provide developers with a concrete set of user-friendly
programming tools that can be used now, while allowing flexibility in
choosing runtime implementations and optimizations that can be applied
to future architectures and runtimes.

FleCSI's control and execution models provide formal nomenclature
for describing poorly understood concepts such as kernels and tasks.
FleCSI's data model provides a low-buy-in approach that makes it an
attractive option for many application projects, as developers are
not locked into particular layouts or data structure representations.

.. toctree::
  :caption: Contents:

  src/build
  src/tutorial
  src/user-guide
  src/developer-guide
  src/team

.. .. doxygenindex::
   This is broken right now.
