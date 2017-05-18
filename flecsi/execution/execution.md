<!-- CINCHDOC DOCUMENT(User Guide) SECTION(Execution Model) -->

# Execution Model

--------------------------------------------------------------------------------

<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Execution Model) -->

# Execution Model

The FleCSI execution model is implemented through a combination of macro
and C++ interfaces. Macros are used to implement the high-level user
interface through calls to the core C++ interface. The general structure
of the code is illustrated in figure FIXME.

The primary interface classes context\_t, task\_model\_t, and
function\_t are types that are defined during configuration that encode
the low-level runtime that was selected for the build. These types are
then used in the macro definitions to implement the high-level FleCSI
interface. Currently, FleCSI supports a serial runtime, and the MPI and
Legion distributed-memory runtimes. The code to implement the backend
implementations for each of these is in the respectively named
sub-directory of *execution*, e.g., the serial implementation is in
*serial*. Documentation for the macro and core C++ interfaces is
maintained in the Doxygen documentation. 

**Note:** Compile-time selection of the low-level runtime is handled
by the pre-processor through type definition in files of the form
*flecsi\_runtime\_X*, where *X* is a policy or runtime model, e.g.,
flecsi\_runtime\_context\_policy, or flecsi\_runtime\_execution\_policy.
These should be updated when new backend support is added or when a
runtime is removed. The files are located in the *flecsi* sub-directory.

## Tasks

FleCSI tasks are *pure* functions, i.e., pure functions with controlled
side-effects.

## Functions

## Kernels

## FleCSIT

FleCSI offers two primary styles of development: The *FleCSIT*
compilation tool that allows fast prototyping of multi-physics ideas,
and the application interface that is intended for production code
development.

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
