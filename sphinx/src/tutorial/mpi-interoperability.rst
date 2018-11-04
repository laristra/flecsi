Example 8: MPI Interoperability
===============================

FleCSI provides straightforward interoperability with libraries and code
that depend on the MPI runtime. Data that are registered in FleCSI may
be passed to an MPI task and used as normal. This example demonstrates
accessing data that have been registered and initialized in FleCSI from
the MPI runtime. Additionally, this example demonstrates a mechanism for
creating persistent references from FleCSI to data that are allocated in
the MPI runtime.

NOTES:

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
