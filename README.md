# FleCSI Project

FleCSI is a set of computer science infrastructure tools designed to aid in the implementation of
multi-physics application development.

# Getting the Code

FleCSI uses git submodules, so it must be checked out recursively:

    % git clone --recursive git@gitlab.lanl.gov:csse/flecsi.git

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

Next, set the TPL\_INSTALL\_PREFIX environment variable to the path you specified above.

Now, you can configure FleCSI using the **arch/developer-gnu** script:

    % mkdir build
	 % cd build
	 % ../arch/developer-gnu
	 % make
	 % make test
