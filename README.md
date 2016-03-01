![logo](config/flecsi.png)

# FleCSI Project

FleCSI is a set of computational science infrastructure tools designed to aid in the implementation of
multi-physics application development.

# Getting the Code

FleCSI uses git submodules, so it must be checked out recursively:

    % git clone --recursive git@github.com:flecsi/flecsi.git

# Requirements

The primary requirement to build FleCSI is a C++14 capable compiler.
Currently, this equates to G++ 5.0 or greater.

* CMake >= 3.0
* GCC >= 5.0
* Doxygen >= 1.8
* cinch-utils 1.0

cinch-utils is available [here](https://github.com/losalamos/cinch-utils).

# Developer Instructions

To begin, you will need to build the third-party library dependencies:

    % git clone git@github.com:flecsi/third-party.git
    % cd flecsi-thirdparty
    % mkdir build
    % cd build
    % ccmake ..

At this point, you should enable Exodus (ENABLE\_EXODUS) and set the install
path to a directory where you have write permissions.

Now build the third-party libraries:

    % make

This command will build and install the libraries in the prefix that
you have specified.

Next, set the TPL\_INSTALL\_PREFIX environment variable to the path you
specified above:

    % export TPL_INSTALL_PREFIX=/path/to/thirdparty/install/directory

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

# Release

This software has been approved for open source release and has
been assigned **LA-CC-15-000**.

# Copyright

Copyright (c) 2016, Los Alamos National Security, LLC
All rights reserved.

Copyright 2016. Los Alamos National Security, LLC. This software was produced under U.S. Government contract DE-AC52-06NA25396 for Los Alamos National Laboratory (LANL), which is operated by Los Alamos National Security, LLC for the U.S. Department of Energy. The U.S. Government has rights to use, reproduce, and distribute this software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.  If software is modified to produce derivative works, such modified software should be clearly marked, so as not to confuse it with the version available from LANL.
 
Additionally, redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:  

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Los Alamos National Security, LLC, Los Alamos National Laboratory, LANL, the U.S. Government, nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

<!-- vim: set tabstop=4 shiftwidth=4 expandtab : -->
