![logo](doc/flecsi.png)

[![Build Status](https://travis-ci.com/laristra/flecsi.svg?branch=master)](https://travis-ci.com/laristra/flecsi)
[![codecov.io](https://codecov.io/github/laristra/flecsi/coverage.svg?branch=master)](https://codecov.io/github/laristra/flecsi?branch=master)

# Introduction

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

# Developers

If you are doing development of FleCSI, please take some time to read
the developer [README](developer/README.md).

# Requirements

The primary requirement for building FleCSI is that you have
a C++17-capable compiler.

## Tools

You'll need the following tools in order to build FleCSI:

   * Boost >= 1.59
   * CMake >= 3.12
   * GCC >= 7.3.0

Install tools in the customary manner for your machine, e.g. by using
*apt-get* on a Ubuntu system, or *dnf* for Fedora.

## Documentation

For documentation, you'll need these as well:

* Doxygen >= 1.8
* [cinch-utils](https://github.com/laristra/cinch-utils) >= 1.0
* [Pandoc](https://pandoc.org) >= 1.19

## Darwin

If you wish to build FleCSI on LANL's Darwin cluster, see the Darwin
Cluster section later in this document.

# Installing the FleCSI Third-Party Libraries

Before installing FleCSI, you should install the FleCSI third-party
libraries. We'll assume that you wish to install the FleCSI third-party
libraries in your home directory.

## Spack

Here we talk about how to install those dependencies through Spack.

Setup your environment and load your modules such as
```
$ module purge
$ module load gcc/8.3.0
$ module load python/3.5.1
```

First, you need to download Spack if you don't already have one.
```
$ git clone https://github.com/spack/spack.git
```
Then do
```
$ source spack/share/spack/setup-env.sh
$ spack compiler find
==> Added 2 new compilers to /home/<user>/.spack/linux/compilers.yaml
    gcc@8.3.0  gcc@4.8.5
==> Compilers are defined in the following files:
    /home/<user>/.spack/linux/compilers.yaml

$ spack compiler list
==> Available compilers
-- gcc centos7-x86_64 -------------------------------------------
gcc@8.3.0  gcc@4.8.5
```
to get Spack into your environment and see what compilers you have that
Spack can find automatically. 

Get FleCSI on your desired branch following the
[Download](#download) section under *Installing FleCSI*.

Next, add the folder that contains custom flecsi spackage to Spack (you
should be in flecsi folder if you followed the steps in the Download
section)
```
$ spack repo add spack-repo
==> Added repo with namespace 'lanl_ristra_flecsi'.

$ spack repo list
==> 2 package repositories.
laristra_flecsi    /home/<user>/flecsi/spack-repo
builtin            /home/<user>/spack/var/spack/repos/builtin
```
Now, assuming you have the compiler you want recognized by Spack
and added the folder, you could just do the install for a legion backend
using mpich like this
```
$ spack install -v --only dependencies flecsi%gcc@8.3.0 +hdf5 backend=legion ^mpich@3.2.1%gcc@8.3.0
```
to get all the dependencies and all their dependencies installed from
scratch.

[NOTE: For internal developers and users, you might need to download tar
files for spack.] 

After Spack finishes the installation, you can load them into
your environment by doing
```
$ spack build-env --dump flecsi-deps.sh "flecsi%gcc@8.3.0 +hdf5 backend=legion ^mpich@3.2.1%gcc@8.3.0"
$ source flecsi-deps.sh
```
But if you want to save time or there is some packages that spack
has trouble installing, you could let Spack know what
packages or modules you already have on the system by adding
`packages.yaml` to your `~/.spack`, which could look something like this
```
packages:
    perl:
        paths:
            perl@5.16.3: /usr
    numactl:
        paths:
            numactl@system: /usr
    python:
        modules:
            python@2.7.3: python/2.7.3
            python@3.5.1: python/3.5.1
    mpich:
        modules:
           mpich@3.2.1%gcc@7.3.0: mpich/3.2.1-gcc_7.3.0
    openmpi:
        modules:
            openmpi@3.1.3%gcc@7.3.0: openmpi/3.1.3-gcc_7.3.0
    cmake:
        modules:
            cmake@3.12.4: cmake/3.12.4
```
Then the installation from Spack will take less time.

Either way, you can proceed to [Build](#build) section under
*Installing FleCSI*.

## Build from Source

Begin by downloading the FleCSI third-party libraries:
```
$ cd
$ git clone --recursive https://github.com/laristra/flecsi-third-party.git
```

Next, enter *flecsi-third-party* and make a build directory:
```
$ cd flecsi-third-party
$ mkdir build
$ cd build
```
Then, for example, you can do the following for a debug-mode build:
```
$ cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=$HOME/flecsi-third-party-debug/
```
Alternatively, you can run *ccmake* in place of *cmake*, and use
*ccmake*'s interface to set the options.

Finally:
```
$ make
$ make install
```
builds and installs the FleCSI third-party libraries in the prefix
that you specified.

# Installing FleCSI

Now that the FleCSI third-party libraries are installed, you can
download and build FleCSI itself. As with the third-party libraries,
we'll assume that you wish to install FleCSI in your home directory.

## Download

First, download FleCSI from GitHub:
```
$ cd
$ git clone --recursive https://github.com/laristra/flecsi.git
```
By default, you'll be on the *master* branch. Let's say you wish to
work in the *branchname* branch instead. Enter *flecsi*, switch to
the relevant branch, and be sure that you have the latest updates:
```
$ cd flecsi
$ git checkout branchname   # if you wish to work in this branch
$ git pull
$ git submodule update --recursive
```

## Build

You can now build FleCSI. For example, a Debug build using Legion
can be done like this:
```
$ mkdir build
$ cd build
$ cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_UNIT_TESTS=ON \
    -DFLECSI_RUNTIME_MODEL=legion
$ make
```
where you should add *cmake*'s *-DCMAKE_PREFIX_PATH* that points to
your FleCSI third-party library installation if you built
from source.

Again, you can run *ccmake* in place of *cmake*.

## Building the Documentation

You can build the FleCSI User and Developer Guides, as well as the
Doxygen interface documentation by specifying additional arguments to
the CMake configuration line:
```
$ cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_UNIT_TESTS=ON \
    -DFLECSI_RUNTIME_MODEL=legion \
    -DENABLE_SPHINX=ON \
    -DENABLE_DOXYGEN=ON
$ make
```
This will build the User and Developer Guides in the *doc* subdirectory.

In order to build the Doxygen interface documentation, you will need to
execute:
```
$ make doxygen 
$ make sphinx
```
which will build the interface documentation (also under the *doc*
subdirectory).

## Test

From within the *build* directory, and after running *make*
as described above, you can run
```
$ make test
```
to run FleCSI's unit tests.

# Workflow

FleCSI uses the
[GitHub Flow](https://guides.github.com/introduction/flow)
workflow pattern.

When you check-out FleCSI, you'll be on the *master* branch. This is
the FleCSI development branch, which is protected in order to ensure
that it is always deployable.

New work should always be done on a separate feature branch.
When you have finished making updates to your branch, you should submit
a pull request. Your changes will be tested for compliance, and reviewed
by the maintainers of the project. If your changes are accepted, they
will be merged into *master*.

# Darwin Cluster

On Darwin, you can simplify some of the build requirements by using
the **ngc/devel-gnu** environment module:
```
$ module load ngc   # ngc/devel-gnu is the default
```
which will load up-to-date compiler and documentation tools.
It will also load a compatible Boost & MPICH libraries.

Note: Boost & MPICH modules should match the *gcc* version and gcc/8.1.0
is automatically loaded.

# Release

This software has been approved for open source release and has
been assigned **LA-CC-16-022**.

# Copyright

Copyright (c) 2016, Triad National Security, LLC.
All rights reserved.

This program was produced under U.S. Government contract 89233218CNA000001 for
Los Alamos National Laboratory (LANL), which is operated by Triad National
Security, LLC for the U.S. Department of Energy/National Nuclear Security
Administration.

All rights in the program are reserved by Triad National Security, LLC, and the
U.S. Department of Energy/National Nuclear Security Administration. The
Government is granted for itself and others acting on its behalf a
nonexclusive, paid-up, irrevocable worldwide license in this material to
reproduce, prepare derivative works, distribute copies to the public, perform
publicly and display publicly, and to permit others to do so.

Additionally, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of Los Alamos National Security, LLC, Los Alamos
   National Laboratory, LANL, the U.S. Government, nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Triad National Security, LLC. AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Triad National Security, LLC. OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
