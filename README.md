![logo](doc/flecsi.png)
[![Build Status](https://travis-ci.org/laristra/flecsi.svg?branch=master)](https://travis-ci.org/laristra/flecsi)

# FleCSI Project

FleCSI is a compile-time configurable framework designed to support
multi-physics application development.  As such, FleCSI attempts to
provide a very general set of infrastructure design patterns that can be
specialized and extended to suit the needs of a broad variety of solver
and data requirements.  Current support includes multi-dimensional mesh
topology, mesh geometry, and mesh adjacency information, n-dimensional
hashed-tree data structures, graph partitioning interfaces, and
dependency closures.

FleCSI also introduces a functional programming model with control,
execution, and data abstractions that are consistent with both MPI and
state-of-the-art task-based runtimes such as Legion and Charm++.  The
FleCSI abstraction layer provides the developer with insulation from the
underlying runtime, while allowing support for multiple runtime systems,
including conventional models like asynchronous MPI.  The intent is to
give developers a concrete set of user-friendly programming tools that
can be used now, while allowing flexibility in choosing runtime
implementations and optimizations that can be applied to architectures
and runtimes that arise in the future.  The control and execution models
in FleCSI also provide formal nomenclature for
describing poorly understood concepts like kernels and tasks.  To
provide a low barrier to entry, the FleCSI data model does not lock
developers into particular layouts or data structure representations,
thus providing a low-buy-in approach that make FleCSI an attractive
option for many application projects.

# Getting the Code

FleCSI uses git submodules, so it must be checked out recursively:

    % git clone --recursive git@github.com:laristra/flecsi.git

https:  

    % git clone --recursive https://github.com/laristra/flecsi.git

# Requirements

The primary requirement to build FleCSI is a C++14 capable compiler.
Currently, this equates to G++ 5.0 or greater.

* CMake >= 3.0
* GCC >= 6.1.1
* Doxygen >= 1.8
* [Cereal](https://github.com/USCiLab/cereal) >= 1.2.1
* [cinch-utils](https://github.com/laristra/cinch-utils) >= 1.0

## Legion

FleCSI has been tested against *update hash here*.

# Developer Instructions

To begin, you will need to build the third-party library dependencies:

    % sudo apt-get install liblapacke-dev libscotch-dev libmetis-dev libexodusii-dev 
    
or build them yourself:

    % git clone git@github.com:laristra/flecsi-third-party.git
    % cd flecsi-third-party
    % mkdir build
    % cd build
    % ccmake ..

At this point, you should enable Exodus (ENABLE\_EXODUS) and set the install
path to a directory where you have write permissions.

Now build the third-party libraries:

    % make

This command will build and install the libraries in the prefix that
you have specified.

Next, set the CMAKE\_PREFIX\_PATH environment variable to the path you
specified above:

    % export CMAKE_PREFIX_PATH=/path/to/thirdparty/install/directory

Now, you can configure FleCSI using the ***arch/developer-gnu*** script:

    % mkdir build
    % cd build
    % ../arch/developer-gnu
    % make
    % make test

**Darwin Build**

On Darwin, you can simplify some of the build requirements by using the
***ngc/devel-gnu*** environment module:

    % module load ngc (devel-gnu is the default)

This will load up-to-date compiler and documentation tools.

## Workflow

FleCSI uses the [GitHub Flow](https://guides.github.com/introduction/flow)
workflow pattern.

When you check-out FleCSI, you will be on the master branch. This is the
FleCSI development branch, which is protected to insure that it is always
deployable. New work should be done on a separate feature branch. When
you have finished making updates to your branch, you should submit a pull
request. Your changes will be tested for compliance and reviewed by the
maintainers of the project. If your changes are accepted, they will be
merged into the master branch.

# Release

This software has been approved for open source release and has
been assigned **LA-CC-16-022**.

# Copyright

Copyright (c) 2016, Los Alamos National Security, LLC
All rights reserved.

Copyright 2016. Los Alamos National Security, LLC. This software was produced under U.S. Government contract DE-AC52-06NA25396 for Los Alamos National Laboratory (LANL), which is operated by Los Alamos National Security, LLC for the U.S. Department of Energy. The U.S. Government has rights to use, reproduce, and distribute this software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.  If software is modified to produce derivative works, such modified software should be clearly marked, so as not to confuse it with the version available from LANL.
 
Additionally, redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:  

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Los Alamos National Security, LLC, Los Alamos National Laboratory, LANL, the U.S. Government, nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
