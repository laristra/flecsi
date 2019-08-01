.. |br| raw:: html

   <br />

.. _build:

Building FleCSI
===============

FleCSI can be configured to run with different distributed-memory
runtimes, including Legion, and MPI. FleCSI also has support for various
fine-grained, node-level runtimes, including OpenMP, Kokkos, and
the C++17 extensions for parallelism. Full documentation of FleCSI
requires both Sphinx and Doxygen. These configuration options are listed
to convey to the reader that the FleCSI build system has several paths
that can be taken to tailor FleCSI to a given system and architecture.

Requirements & Prerequisites
****************************

The following list of requirements provides a complete set of build
options, but is not necessary for a particular build:

* **C++17 compliant compiler** |br|
  At the current time, FleCSI has been tested with GNU, Clang, and Intel
  C++ compilers.

* **MPI** |br|
  If Legion support is enabled, the MPI implementation must have support
  for *MPI_THREAD_MULTIPLE*.

* **Legion** |br|
  We are currently using the most up-to-date version of the master
  branch.

* **GASNet** |br|
  GASNet is only required if Legion support is enabled.

* **CMake** |br|
  We currently require CMake version 3.12 or greater.

* **Boost** |br|
  We require *program_options*, *serialization*, and *stacktrace*.

* **Doxygen** |br|
  Doxygen is only required to build the interface documentation.

* **Sphinx** |br|
  Sphinx is only required to build the web-based documentation. We are
  currently using Sphinx 1.1.0. We also require the Sphinx RTD Theme
  (using version 0.4.2). These can be installed on most Linux systems
  using pip.

* **Python** |br|
  We currently require Python 2.7 or greater.

Spack
*****

The preferred method for installing FleCSI is to use the
`spack <https://github.com/spack/spack>`_ package:

.. code-block:: console

  $ spack install flecsi
  $ spack load -r flecsi

Note that unless you specify which compiler to use to build FleCSI,
spack will use the default one on your system. Here is an example how to
explicitly tell spack which compiler to use for building FleCSI:

.. code-block:: console

  $ spack install flecsi%gcc@8.1.0  
 
FleCSI supports several different versions and variants, e.g.:

.. code-block:: console

  $ spack install flecsi@1.0 +legion +graphviz

For a complete list of versions and variants, type:

.. code-block:: console

  $ spack info flecsi

Currently, FleCSI depends on the following spack packages:

.. code-block:: console

  $ spack install cmake
  $ spack install boost@1.68.0
  $ spack install parmetis
  $ spack install hdf5
  $ spack install kokkos
  $ spack install legion@ctrl-rep

.. note::

  Users of spack can safely ignore the remaining sections on this page.
  Loading the flecsi spack package will properly configure your
  environment.

FleCSI Third Party Libraries Project
************************************

.. warning::

  The FleCSI superbuild project is deprecated in favor of Spack, and
  will eventually be removed.

To facilitate FleCSI adoption by a broad set of users, we have provided
a superbuild project that can build many of the libraries and tools
required to build and use FleCSI. The FleCSI Third Party Libraries (TPL)
project is available from github at
`https://github.com/laristra/flecsi-third-party
<https://github.com/laristra/flecsi-third-party>`_.
Note that a suitable version of MPI is required for the superbuild.

.. admonition:: Admonishment

  Users should note that, while this approach is easier, it may not
  provide as robust a solution as individually building each dependency,
  and that care should be taken before installing these libraries on a
  production system to avoid possible conflicts or duplication.
  Production deployments of some of these tools may have architecture or
  system specific needs that will not be met by our superbuild. Users
  who are working with development branches of FleCSI are encouraged to
  build each package separately.

Build instructions for the TPLs:

.. code-block:: console

  $ git clone --recursive https://github.com/laristra/flecsi-third-party.git
  $ cd flecsi-third-party
  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install/directory ..
  $ make
  $ make install

**It is recommended that users remove the install directory before
updating or re-compiling the TPLs to insure a clean build.**

Build Environment
*****************

FleCSI uses CMake as part of its build system. A convenient mechanism
for identifying directory paths that should be searched by CMake's
*find_package* function is to set the *CMAKE_PREFIX_PATH* environment
variable. If you are using the FleCSI TPLs discussed above, you can set
*CMAKE_PREFIX_PATH* to include the path to your TPL installation
directory and FleCSI will automatically find all of its dependencies:

.. code-block:: console

  $ export CMAKE_PREFIX_PATH=/path/to/install/directory (bash)

Getting The Code
****************

Clone the FleCSI git repository, and create an out-of-source build area
(FleCSI prohibits in-source builds):

.. code-block:: console

  $ git clone --recursive https://github.com/laristra/flecsi.git
  $ cd flecsi
  $ mkdir build
  $ cd build

**Note: FleCSI developers, i.e., those who have permission to create
new branches and pull requests, should clone the source code using their
github account and the *git* user:**

.. code-block:: console

  $ git clone --recursive git@github.com:laristra/flecsi.git
  $ cd flecsi
  $ mkdir build
  $ cd build

Configuration & Build
*********************

Configuration of FleCSI requires the selection of the backend runtimes
that will be used by the FleCSI programming model abstraction to invoke
tasks and kernels. There are currently two supported distributed-memory
runtimes, a serial runtime, and one supported node-level runtime:

* **Distributed-Memory** |br|
  Legion or MPI

* **Serial [supported thorugh MPI runtime]** |br|
  **The serial build is no longer supported.** Users wishing to emulate
  this build mode should select the MPI runtime and run executables with
  a single-rank.

* **Node-Level** |br|
  OpenMP

Example configuration: **MPI**

.. code-block:: console

  $ cmake -DFLECSI_RUNTIME_MODEL=mpi ..

Example configuration: **MPI + OpenMP**

.. code-block:: console

  $ cmake -DFLECSI_RUNTIME_MODEL=mpi -DENABLE_OPENMP ..

Example configuration: **Legion**

.. code-block:: console

  $ cmake -DFLECSI_RUNTIME_MODEL=legion ..

After configuration is complete, just use *make* to build:

.. code-block:: console

  $ make -j 16

Installation uses the normal *make install*, and will install FleCSI in
the directory specified by CMAKE_INSTALL_PREFIX:

.. code-block:: console

  $ make install

Building the Unit Tests
***********************

To build FleCSI unit test suite, enable the option for the FleCSI
logging utility (flog). **By default, this will also enable the unit
tests.**

.. code-block:: console

  $ cmake .. -DENABLE_FLOG

After building FleCSI, you can run the unit tests like:

.. code-block:: console

  $ make test

Building the Documentation
**************************

FleCSI uses Doxygen for its API reference, and Sphinx for user and
developer documentation.

Doxygen can be installed with most Linux package managers.  To install
Sphinx, you can install pip3, and use it to install *Sphinx*,
*recommonmark*, and *sphinx_rtd_theme*. Your package manager should also
have pip3, e.g., on Ubuntu, you can install all of these requirements
like:

.. code-block:: console

  $ sudo apt install doxygen
  $ sudo apt install python3-pip
  $ sudo apt install python3-breathe
  $ pip3 install Sphinx
  $ pip3 install recommonmark
  $ pip3 install sphinx_rtd_theme

To enable Doxygen and Sphinx, these options need to be enabled in CMake:

.. code-block:: console

  $ cmake -DENABLE_DOXYGEN=ON -DENABLE_SPHINX=ON ..

Once you have properly configured FleCSI, you can build the
documentation like:

.. code-block:: console

  $ make doxygen
  $ make sphinx

Both of these targets will be built in your build directory under *doc*,
e.g., the main Doxygen index.html page will be located at
*'doc/doxygen/html/index.html'*. Similarly, the Sphinx main index.html
page will be located at *'doc/sphinx/index.html'*. You can open these in
your browser with
*file:///path/to/your/build/directory/doc/doxygen/html/index.html*, and
*file:///path/to/your/build/directory/doc/sphinx/index.html*.

Advanced
********

.. toctree::

  build/options

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
