# FleCSI: Tutorial - 08 MPI Interoperability
<!--
  The above header is required for Doxygen to correctly name the
  auto-generated page. It is ignored in the FleCSI guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(tutorial::mpi-interoperability) -->

# MPI Interoperability Example

FleCSI provides straightforward interoperability with libraries and code
that depend on the MPI runtime. Data that are registered in FleCSI may
be passed to an MPI task and used as normal. This example demonstrates
accessing data that have been registered and initialized in FleCSI from
the MPI runtime. Additionally, this example demonstrates a mechanism for
creating persistent references from FleCSI to data that are allocated in
the MPI runtime.

NOTES:

```cpp
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
