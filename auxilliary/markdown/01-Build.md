# FleCSI: Build
<!--
  The above header ("FleCSI: Build") is required for Doxygen to
  correctly name the auto-generated page. It is ignored in the FleCSI
  guide documentation.
-->

<!-- CINCHDOC DOCUMENT(User Guide) SECTION(Build) -->

# Building FleCSI

FleCSI can be configured to run with different distributed-memory
runtimes, including Legion, and MPI. FleCSI also has support for various
fine-grained, node-level runtimes, including OpenMP, Kokkos, Agency, and
the C++17 extensions for parallelism.  Full documentation of FleCSI
requires both Pandoc and Doxygen. These configuration options are listed
to convey to the reader that the FleCSI build system has several paths
that can be taken to tailor FleCSI to a given system and architecture.

## Requirements & Prerequisites

The following list of requirements provides a complete set of build
options, but is not necessary for a particular build:

* **C++14 compliant compiler**<br>
  At the current time, FleCSI has been tested with GNU, Clang, and Intel
  C++ compilers.

* **MPI**<br>
  If Legion support is enabled, the MPI implementation must have support
  for *MPI_THREAD_MULTIPLE*.

* **Legion**<br>
  We are currently using the most up-to-date version of the master
  branch.

* **GASNet**<br>
  GASNet is only required if Legion support is enabled.

* **Pandoc**<br>
  Pandoc is only required to build the FleCSI guide documentation.
  Pandoc is a format conversion tool. More information is available
  [here](http://pandoc.org).

* **Doxygen**<br>
  Doxygen is only required to build the interface documentation.

* **CMake**<br>
  We currently require CMake version 2.8 or greater.

## FleCSI Third Party Libraries Project

To facilitate FleCSI adoption by a broad set of users, we have provided
a superbuild project that can build many of the libraries and tools
required to build and use FleCSI. The FleCSI Third Party Libraries (TPL)
project is available from github
[here](https://github.com/laristra/flecsi-third-party). Note that a
suitable version of MPI is required for the superbuild.

**Admonishment: Users should note that, while this approach is easier,
it may not provide as robust of a solution as individually building each
dependency, and that care should be taken before installing these
libraries on a production system to avoid possible conflicts or
duplication. Production deployments of some of these tools may have
architecture or system specific needs that will not be met by our
superbuild. Users who are working with development branches of FleCSI
are encouraged to build each package separately.**

Build instructions for the TPLs:
```
% git clone --recursive https://github.com/laristra/flecsi-third-party.git
% cd flecsi-third-party
% mkdir build
% cd build
% cmake -DCMAKE_INSTALL_PREFIX=/path/to/install/directory ..
% make
```
The *make* command will also install the TPLs in the specified install
directory. **It is recommended that users remove the install directory
before updating or re-compiling the TPLs.**

## Build Environment

FleCSI uses CMake as part of its build system. A convenient mechanism
for identifying directory paths that should be searched by CMake's
*find_package* function is to set the *CMAKE_PREFIX_PATH* environment
variable. If you are using the FleCSI TPLs discussed above, you can set
*CMAKE_PREFIX_PATH* to include the path to your TPL installation
directory and FleCSI will automatically find all of its dependencies:
```
% export CMAKE_PREFIX_PATH=/path/to/install/directory (bash)
```

## Getting The Code

Clone the FleCSI git repository, and create an out-of-source build area
(FleCSI prohibits in-source builds):
```
% git clone --recursive https://github.com/laristra/flecsi.git
% cd flecsi
% mkdir build
% cd build
```
**Note: FleCSI developers, i.e., those who have permission to create
new branches and pull requests, should clone the source code using their
github account and the *git* user:**
```
% git clone --recursive git@github.com:laristra/flecsi.git
% cd flecsi
% mkdir build
% cd build
```

## Configuration & Build

Configuration of FleCSI requires the selection of the backend runtimes
that will be used by the FleCSI programming model abstraction to invoke
tasks and kernels. There are currently two supported distributed-memory
runtimes, a serial runtime, and one supported node-level runtime:

* **Distributed-Memory**: Legion or MPI
* **Serial**: This is useful for development and debugging, and is
  logically a valid selection for the distributed-memory backend that is
  not distributed ;-)
* **Node-Level**: OpenMP

Example configuration: **Serial**
```
% cmake -DFLECSI_RUNTIME_MODEL=serial ..
```
Example configuration: **Serial + OpenMP**
```
% cmake -DFLECSI_RUNTIME_MODEL=serial -DENABLE_OPENMP ..
```
Example configuration: **Legion**
```
% cmake -DFLECSI_RUNTIME_MODEL=legion -DENABLE_MPI -DENABLE_PARTITIONING ..
```
After configuration is complete, just use *make* to build:
```
% make -j 16
```
This will build all targets *except* for the Doxygen documentation, which
can be built with:
```
% make doxygen
```
Installation uses the normal *make install*, and will install FleCSI in
the directory specified by CMAKE_INSTALL_PREFIX:
```
% make install
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
