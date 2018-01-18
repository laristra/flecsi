# FleCSI: Tutorial
<!--
  The above header ("FleCSI: Tutorial") is required for Doxygen to
  correctly name the auto-generated page. It is ignored in the FleCSI
  guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(tutorial) -->

# Introduction

This tutorial attempts to give a basic overview of the design philosohpy
and structure of the FleCSI programming system. Each subdirectory
contains an example code that can be compiled with the *flecsit*
compiler script. More details on how to do this are given below in the
*Getting Started* section.

# FleCSI Design Philosophy

The structure of applications built on top of the FleCSI programming
system assumes three basic types of users. Each of the user types has
their own set of responsibilities that are designed to separate
concerns, and to make sure that development tasks are intuitive and
acheivable by the associated user type.

The user types are:

* **FleCSI Core Developer**<br>  
These are users who design, implement, and maintain the core FleCSI
library. Generally, these users are expert C++ developers who have a
well-developed understanding of the the low-level design of the FleCSI
software architecture. These users are generally computer scientists
with expertise in generic programming techniques, data structure design,
and optimization.

* **FleCSI Specialization Developer**<br>  
These are users who adapt the core FleCSI data structures and runtime
interfaces to create domain-specific interfaces for application
developers.  These users are required to understand the components of
the FleCSI interface that can be statically specialized, and must have a
solid understanding of the runtime interface. Additionally,
specialization develoeprs are assumed to understand the requirements of
the application area for which they are designing an interface. These
users are generally computational scientists with expertise in one or
more numerical methods areas.

* **FleCSI Application Developer**<br>  
These users are methods developers or physiciests who use a particular
FleCSI specialization layer to develop and maintain application codes.
These are the FleCSI end-users, who have expertise in designing and
implementing numerical methods to solve complicated, multiphysics
simulation problems.

The source code implementing a FleCSI project will reflect this user
structure: The project will link to the core FleCSI library; The project
will use one or more specializations (These will usually also be
libraries that are linked to by the application.); and, The application
developers will use the core and specialization interfaces to write
their applications.

# Tutorial

This tutorial is primarily designed as an introduction for application
developers, i.e., we do not go into the details of designing or
implmenting the specialization layer, and the discussion of core FleCSI
features is limited to the high-level execution and data interfaces.

Because a specialization layer is necessary to use FleCSI, we have
provided a simple mesh interface as part of the tutorial. Users who are
interested in the basic structure of a mesh_topology specialization are
encouraged to examine the source code in the *specialization*
subdirectory of this tutorial (The complete source code is in the
*flecsi-tutorial/specialization* subdirectory of the main project.)

# Requirements

This tutorial assumes that you have successfully downloaded and built
FleCSI, and that you have installed it somewhere on your system.
Instructions for building FleCSI are available from the related pages
[here](pages.html).

# Building the Examples

The example codes in the tutorial are meant to be built using the
*flecsit* tool. To use *flecsit*, you must first configure your path to
find the script, and to include any dynamic library dependencies. We
have provided initialization scripts for bash, csh, and environment
modules to ease this step.

To use the bash or csh script, simply source the script (located in the
bin directory of your FleCSI install path):

```
$ source CMAKE_INSTALL_PREFIX/bin/flecsi-tutorial.{sh,csh}
```

To use the environment module, you will need to install the module file
in an appropriate path (This may have been done by your administrator.)
You can then load the module as usual:

```
$ module load flecsi-tutorial
```

Once your environment has been correctly configured, you can build any
of the tutorial examples like:

```
$ flecsit compile example.cc
```

where *example.cc* is the name of the example source file. This will
produce an executable that can be run like:

```
$ ./example.Linux
```

Note that the executable suffix will depend on your operating system.
Some exmaples are designed to run in parallel (indicated in the example
documentation). These should be run with a suitable MPI interpreter:

```
$ mpirun -np 2 example.Linux
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
