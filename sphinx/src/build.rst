.. |br| raw:: html

   <br />

.. _build:

Build & Install
===============

FleCSI can be configured to run with different distributed-memory
runtimes, including Legion, and MPI. FleCSI also has support for various
fine-grained, node-level runtimes, including OpenMP, Kokkos, and
the C++17 extensions for parallelism. Full documentation for FleCSI
requires both Sphinx and Doxygen. These configuration options are listed
to convey to the reader that the FleCSI build system has several paths
that can be taken to tailor FleCSI to a given system and architecture.

Requirements & Prerequisites
****************************

The following list of requirements provides a complete set of build
options, but is not necessary for a particular build:

.. note::

  CI listings indicate specific versions used by FleCSI's continuous
  integration tests. If nothing is indicated, there is no specific
  version tested.

* **Boost** |br|
  We require *program_options* and *stacktrace*. |br|
  *CI: 1.70.0*

* **C++17 compliant compiler** |br|
  At the current time, FleCSI has been tested with GNU, Clang, and Intel
  C++ compilers. |br|
  *CI: gcc 8.3.0, clang 8.0.1, icpc 19.0.2*

* **Doxygen** |br|
  Doxygen is only required to build the interface documentation.

* **GASNet** |br|
  GASNet is only required if Legion support is enabled.

* **MPI** |br|
  If Legion support is enabled, the MPI implementation must have support
  for *MPI_THREAD_MULTIPLE*. |br|
  *CI: mpich 3.2.1, openmpi 3.1.3*

* **Legion** |br|
  We are currently using the most up-to-date version of the control
  replication branch. |br|
  *CI: control_replication branch*

* **parMETIS/METIS** |br|
  *CI: 4.0.3 (parMETIS), 5.1.0 (METIS)*

* **CMake** |br|
  We currently require CMake version 3.12 or greater.
  *CI: 3.12*

* **Sphinx** |br|
  Sphinx is only required to build the web-based documentation. We are
  currently using Sphinx 1.1.0. We also require the Sphinx RTD Theme
  (using version 0.4.2). These can be installed on most Linux systems
  using pip.

* **Python** |br|
  We currently require Python 3.0 or greater.

Getting The Code
****************

.. note::

  If you are a user and only want to install FleCSI, you can skip this
  step and use the instructions for installing FleCSI using Spack.

Clone the FleCSI git repository, and create an out-of-source build area
(FleCSI prohibits in-source builds):

.. code-block:: console

  $ git clone https://github.com/laristra/flecsi.git
  $ cd flecsi
  $ mkdir build
  $ cd build

Spack
*****

The preferred method for installing FleCSI and its dependencies is to
use `Spack <https://github.com/spack/spack>`_. Spack is easy
to install and configure:

.. code-block:: console

  $ git clone git@github.com:spack/spack.git
  $ source path/to/spack/repository/share/spack/setup-env.sh

Once spack is installed, you can install FleCSI like:

.. code-block:: console

  $ spack install flecsi

FleCSI supports several different versions and variants, e.g.:

.. code-block:: console

  $ spack install flecsi@1.0 +legion +graphviz

For a complete list of versions and variants, type:

.. code-block:: console

  $ spack info flecsi

More documentation and information on Spack is available `here
<https://spack.readthedocs.io/en/latest>`_.

FleCSI Developers
*****************

If you are a developer, and would like to install only the dependencies of
FleCSI (assuming that you will build FleCSI from source), you can use
spack's *--only* option:

.. code-block:: console

  $ spack install --only dependencies flecsi backend=legion +hdf5 ^mpich

If you are developing against a particular branch of FleCSI, you can
capture branch-specific spack dependencies by adding the FleCSI spack
repo (before performing the above step):

.. code-block:: console

  $ spack repo add path/to/flecsi/spack-repo

This will prepend a spack repository path to your spack configuration,
such that the specific branch of FleCSI can override the normal builtin
spack dependencies to provide whatever features are required for a
successful build.

Configuration & Build
*********************

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

  $ cmake .. -DENABLE_FLOG=ON

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
