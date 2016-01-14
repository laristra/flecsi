# FleCSI Project

FleCSI is a set of computer science infrastructure tools designed to aid in the implementation of
multi-physics application development.

# Getting the Code

FleCSI uses git submodules, so it must be checked out recursively:

    % git clone --recursive git@gitlab.lanl.gov:csse/flecsi.git

# Requirements

The primary requirement to build FleCSI is a C++14 capable compiler.  Currently,
this equates to G++ 5.0 or greater.

* CMake >= 3.0
* GCC >= 5.0
* Doxygen >= 1.8
* Cinch-utils 1.0

Cinch-utils is available [here](http://gitlab.lanl.gov/csse/cinch-utils).

# Developer Instructions

To begin, you will need to build the third-party library dependencies:

    % git clone git@gitlab.lanl.gov:csse/thirdparty.git
    % cd thirdparty
    % mkdir build
    % cd build
    % ccmake ..

At this point, you should enable Exodus (ENABLE\_EXODUS) and set the install path to a
directory where you have write permissions.

Now build the third-party libraries:

    % make

This command will build and install the libraries in the prefix that you have specified.

Next, set the TPL\_INSTALL\_PREFIX environment variable to the path you specified above:

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

<!-- vim: set tabstop=4 shiftwidth=4 expandtab : -->
