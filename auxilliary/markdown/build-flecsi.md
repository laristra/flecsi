<!-- CINCHDOC DOCUMENT(User Guide) SECTION(Build) -->

# Building FleCSI

FleCSI can be configured to run with several different
distributed-memory runtimes, including Legion, and MPI. FleCSI also has
support for various fine-grained node-level runtimes, including OpenMP,
Kokkos, Agency, and the C++17 extensions for parallelism. Full
documentation of FleCSI requires both Pandoc and Doxygen. These
configuration options are listed to convey to the reader that the FleCSI
build system has several paths through it that can be used to tailor
FleCSI to a given system and architecture. The following list of
requirements provides a complete set of build options, but is not
necessary for certain specific builds:

* **C++14 complient compiler**

* **MPI**
  If Legion support is needed, the MPI implementation must have support
  for MPI_THREAD_MULTIPLE


--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
