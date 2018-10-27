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
  at [http://pandoc.org](http://pandoc.org).

* **Doxygen**<br>
  Doxygen is only required to build the interface documentation.

* **CMake**<br>
  We currently require CMake version 2.8 or greater.

* **Python**<br>
  We currently require Python 2.7 or greater.

## FleCSI Third Party Libraries Project

To facilitate FleCSI adoption by a broad set of users, we have provided
a superbuild project that can build many of the libraries and tools
required to build and use FleCSI. The FleCSI Third Party Libraries (TPL)
project is available from github at
[https://github.com/laristra/flecsi-third-party](https://github.com/laristra/flecsi-third-party).
Note that a suitable version of MPI is required for the superbuild.

**Admonishment: Users should note that, while this approach is easier,
it may not provide as robust a solution as individually building each
dependency, and that care should be taken before installing these
libraries on a production system to avoid possible conflicts or
duplication. Production deployments of some of these tools may have
architecture or system specific needs that will not be met by our
superbuild. Users who are working with development branches of FleCSI
are encouraged to build each package separately.**

Build instructions for the TPLs:
```
$ git clone --recursive https://github.com/laristra/flecsi-third-party.git
$ cd flecsi-third-party
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install/directory ..
$ make
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
$ export CMAKE_PREFIX_PATH=/path/to/install/directory (bash)
```

## Getting The Code

Clone the FleCSI git repository, and create an out-of-source build area
(FleCSI prohibits in-source builds):
```
$ git clone --recursive https://github.com/laristra/flecsi.git
$ cd flecsi
$ mkdir build
$ cd build
```
**Note: FleCSI developers, i.e., those who have permission to create
new branches and pull requests, should clone the source code using their
github account and the *git* user:**
```
$ git clone --recursive git@github.com:laristra/flecsi.git
$ cd flecsi
$ mkdir build
$ cd build
```

## Configuration & Build

Configuration of FleCSI requires the selection of the backend runtimes
that will be used by the FleCSI programming model abstraction to invoke
tasks and kernels. There are currently two supported distributed-memory
runtimes, a serial runtime, and one supported node-level runtime:

* **Distributed-Memory**<br>
  Legion or MPI

* **Serial [supported thorugh MPI runtime]**<br>
  **The serial build is no longer supported.** Users wishing to emulate
  this build mode should select the MPI runtime and run executables with
  a single-rank.

* **Node-Level**<br>
  OpenMP

Example configuration: **MPI**
```
$ cmake -DFLECSI_RUNTIME_MODEL=mpi -DENABLE_MPI -DENABLE_COLORING ..
```
Example configuration: **MPI + OpenMP**
```
$ cmake -DFLECSI_RUNTIME_MODEL=mpi -DENABLE_MPI -DENABLE_COLORING -DENABLE_OPENMP ..
```
Example configuration: **Legion**
```
$ cmake -DFLECSI_RUNTIME_MODEL=legion -DENABLE_MPI -DENABLE_COLORING ..
```
After configuration is complete, just use *make* to build:
```
$ make -j 16
```
This will build all targets *except* for the Doxygen documentation, which
can be built with:
```
$ make doxygen
```
Installation uses the normal *make install*, and will install FleCSI in
the directory specified by CMAKE_INSTALL_PREFIX:
```
$ make install
```

## CMake Configuration Options

The following set of options are available to control how FleCSI is
built.

* **BUILD_SHARED_LIBS [default: ON]**<br>
  Build shared library objects (as opposed to static).

* **CMAKE_BUILD_TYPE [default: Debug]**<br>
  Specify the build type (configuration) statically for this build tree.
  Possible choices are *Debug*, *Release*, *RelWithDebInfo*, and
  *MinSizeRel*.

* **CMAKE_INSTALL_PREFIX [default: /usr/local]**<br>
  Specify the installation path to use when *make install* is invoked.

* **CXX_CONFORMANCE_STANDARD [default: c++14]**<br>
  Specify to which C++ standard a compiler must conform. This is a
  developer option used to identify whether or not the selected C++
  compiler will be able to compile FleCSI, and which (if any) tests it
  fails to compile. This information can be shared with vendors to
  identify features that are required by FleCSI that are not
  standards-compliant in the vendor's compiler.

* **ENABLE_BOOST_PREPROCESSOR [default: ON]**<br>
  Boost.Preprocessor is a header-only Boost library that provides
  enhanced pre-processor options and manipulation, which are not
  supported by the standard C preprocessor. Currently, FleCSI uses the
  preprocessor to implement type reflection.

* **ENABLE_BOOST_PROGRAM_OPTIONS [default: OFF]**<br>
  Boost.Program\_options provides support for handling command-line
  options to a program. When this build option is enabled, CMake will
  attempt to locate a valid installation of the program options library,
  and Cinch will enable certain command-line options for unit tests. In
  particular, if Cinch's clog extensions are enabled, the *--tags*
  command-line option will be available to select output tags.

* **ENABLE_CINCH_DEVELOPMENT [default: OFF]**<br>
  If this option is enabled, extra information will be generated to help
  debug different Cinch behaviors. Currenlty, this only affects the
  generation of documentation: when enabled, the resulting PDF
  documentation will be annotated with the original locations of the
  content. **FIXME: We should consider renaming this option**

* **ENABLE_CINCH_VERBOSE [default: OFF]**<br>
  If this option is enabled, extra information will be output during the
  CMake configuration and build that may be helpful in debugging Cinch.

* **ENABLE_CLOG [default: OFF]**<br>
  Enable Cinch Logging (clog). The Cinch logging interface provides
  methods for generating and controlling output from a running
  application.

* **CLOG_COLOR_OUTPUT [default: ON]**<br>
  Enable colorization of clog output.

* **CLOG_DEBUG [default: OFF]**<br>
  Enable verbose debugging output for clog.

* **CLOG_ENABLE_EXTERNAL [default: OFF]**<br>
  The Cinch clog facility is a runtime. As such, some of the features
  provided by clog require initialization. Because of the C++ mechanism
  used by clog to implement parts of its interface, it is possible to
  call the interface from parts of the code that are *external*, i.e.,
  at file scope. Externally scoped statements are executed before the
  clog runtime can be initialized, and therefore their output cannot be
  controlled by the clog tagging feature. This option allows the user to
  enable this type of output, which can be quite verbose.

* **CLOG_ENABLE_TAGS [default: OFF]**<br>
  Enable the tag feature for clog. If enabled, users can selectively
  control clog output by specifying active tags on the command line:
```
$ ./executable --tags=tag1,tag2
```
  Invoking the *--tags* flag with no arguments will list the available
  tags. **This option requries that ENABLE_BOOST_PROGRAM_OPTIONS be ON.**

* **CLOG_STRIP_LEVEL [default: 0]**<br>
  Strip levels are another mechanism to allow the user to control the
  amount of output that is generated by clog. In general, the higher the
  stip level, the fewer the number of clog messages that will be output.
  There are five strip levels in clog: *trace*, *info*, *warn*, *error*,
  and *fatal*. Output for all of these levels is turned on if the strip
  level is 0. As the strip level is increased, fewer levels are output,
  e.g., if the strip level is 3, only *error* and *fatal* log messages
  will be output. **Regardless of the strip level, clog messages that
  are designated *fatal* will generate a runtime error and will invoke
  std::exit.**

* **ENABLE_COLORING [default: OFF]**<br>
  This option controls whether or not various library dependencies and
  code sections are active that are required for graph partitioning
  (coloring) and distributed-memory parallelism. In general, if you have
  selected a runtime mode that requires this option, it will
  automatically be enabled.

* **ENABLE_COLOR_UNIT_TESTS [default: OFF]**<br>
  Enable coloraization of unit test output.

* **ENABLE_COVERAGE_BUILD [default: OFF]**<br>
  Enable build mode to determine the code coverage of the current set of
  unit tests. This is useful for continuous integration (CI) test analysis.

* **ENABLE_DEVEL_TARGETS [default: OFF]**<br>
  Development targets allow developers to add small programs to the
  FleCSI source code for testing code while it is being developed. These
  programs are not intended to be used as unit tests, and may be added
  or removed as the code evolves.

* **ENABLE_DOCUMENTATION [default: OFF]**<br>
  This option controls whether or not the FleCSI user and developer
  guide documentation is built. If enabled, CMake will generate these
  guides as PDFs in the *doc* subdirectory of the build.

* **ENABLE_DOXYGEN [default: OFF]**<br>
  If enabled, CMake will verify that a suitable *doxygen* binary is
  available on the system, and will add a target for generating
  Doxygen-style interface documentation from the FleCSI source code.
  **To build the doxygen documentation, users must explicitly invoke:**
```
$ make doxygen
```

* **ENABLE_DOXYGEN_WARN [default: OFF]**<br>
  Normal Doxygen output produces many pages worth of warnings. These are
  distracting and overly verbose. As such, they are disabled by default.
  This options allows the user to turn them back on.

* **ENABLE_EXODUS [default: OFF]**<br>
  If enabled, CMake will verify that a suitable Exodus library is
  available on the system, and will enable Exodus functionality in the
  FleCSI I/O interface.

* **ENABLE_FLECSIT [default: OFF]**<br>
  FleCSIT is a command-line utility that simplifies experimental
  development using FleCSI. This can be thought of as a *sandbox* mode,
  where the user can write code that utilizes a particular FleCSI
  specialization and the FleCSI data and runtime models without the
  overhead of a full production code project. This option simply enables
  creation of the FleCSIT executable.

* **ENABLE_JENKINS_OUTPUT [default: OFF]**<br>
  If this options is on, extra meta data will be output during unit test
  invocation that may be used by the Jenkins CI system.

* **ENABLE_MPI_CXX_BINDINGS [default: OFF]**<br>
  This option is a fall-back for codes that actually require the MPI C++
  bindings. **This interface is deprecated and should only be used if it
  is impossible to get rid of the dependency.**

* **ENABLE_OPENMP [default: OFF]**<br>
  Enable OpenMP support. If enabled, the appropriate flags will be
  passed to the C++ compiler to enable language support for OpenMP
  pragmas.

* **ENABLE_OPENSSL [default: OFF]**<br>
  If enabled, CMake will verify that a suitable OpenSSL library is
  available on the system, and will enable the FleCSI checksum
  interface.

* **ENABLE_UNIT_TESTS [default: OFF]**<br>
  Enable FleCSI unit tests. If enabled, the unit test suite can be run
  by invoking:
```
$ make test
```

* **FLECSI_COUNTER_TYPE [default: int32_t]**<br>
  Specify the C++ type to use for the FleCSI counter interface.

* **FLECSI_DBC_ACTION [default: throw]**<br>
  Select the design-by-contract action.

* **FLECSI_DBC_REQUIRE [default: ON]**<br>
  Enable DBC pre/post condition assertions.

* **FLECSI_ID_FBITS [default: 4]**<br>
  Specify the number of bits to be used to represent id flags. This
  option affects the number of entities that can be represented on a
  FleCSI mesh type. The number of bits used to represent entities is
  62-FLECSI_ID_PBITS-FLECSI_ID_FBITS. With the current defaults there
  are 38 bits available to represent entities, i.e., up to 274877906944
  entities can be resolved.

* **FLECSI_ID_PBITS [default: 20]**<br>
  Specify the number of bits to be used to represent partition ids. This
  option affects the number of entities that can be represented on a
  FleCSI mesh type. The number of bits used to represent entities is
  62-FLECSI_ID_PBITS-FLECSI_ID_FBITS. With the current defaults there
  are 38 bits available to represent entities, i.e., up to 274877906944
  entities can be resolved.

* **FLECSI_RUNTIME_MODEL [default: mpi]**<br>
  Specify the low-level runtime model. Currently, *legion* and *mpi* are
  the only valid options.

* **VERSION_CREATION [default: git describe]**<br>
  This options allows the user to either directly specify a version by
  entering it here, or to let the build system provide a version using
  git describe.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
