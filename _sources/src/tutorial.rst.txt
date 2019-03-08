.. |br| raw:: html

   <br />

FleCSI Tutorial
===============

This tutorial attempts to give a basic overview of the design philosophy
and structure of the FleCSI programming system. Each subdirectory
contains an example code that can be compiled with the *flecsit*
compiler script. More details on how to do this are given below in the
*Getting Started* section.

FleCSI Design Philosophy
************************

The structure of applications built on top of the FleCSI programming
system assumes three basic types of users. Each of the user types has
their own set of responsibilities that are designed to separate
concerns, and to make sure that development tasks are intuitive and
achievable by the associated user type.

The user types are:

* **FleCSI Core Developer** |br|
  These are users who design, implement, and maintain the core FleCSI
  library. Generally, these users are expert C++ developers who have a
  well-developed understanding of the the low-level design of the FleCSI
  software architecture. These users are generally computer scientists
  with expertise in generic programming techniques, data structure
  design, and optimization.
* **FleCSI Specialization Developer** |br|
  These are users who adapt the core FleCSI data structures and runtime
  interfaces to create domain-specific interfaces for application
  developers.  These users are required to understand the components of
  the FleCSI interface that can be statically specialized, and must have
  a solid understanding of the runtime interface. Additionally,
  specialization developers are assumed to understand the requirements
  of the application area for which they are designing an interface.
  These users are generally computational scientists with expertise in
  one or more numerical methods areas.
* **FleCSI Application Developer** |br|
  These users are methods developers or physicists who use a particular
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

Admonishment
************

This tutorial is primarily designed as an introduction for application
developers, i.e., we do not go into the details of designing or
implementing the specialization layer, and the discussion of core FleCSI
features is limited to the high-level execution and data interfaces.

Because a specialization layer is necessary to use FleCSI, we have
provided a simple mesh interface as part of the tutorial. Users who are
interested in the basic structure of a mesh_topology specialization are
encouraged to examine the source code in the *specialization*
subdirectory of this tutorial (The complete source code is in the
*flecsi-tutorial/specialization* subdirectory of the main project.)

Requirements
************

This tutorial assumes that you have successfully downloaded and built
FleCSI, and that you have installed it somewhere on your system.
Instructions for building FleCSI are available here: :ref:`build`

Using the Docker Container
**************************

The tutorial is also available as a Docker container. This simplifies
getting and installing FleCSI and its dependencies. To use the Docker
container, you should have the Docker daemon installed on your system.
Docker is available for several different platforms from the
`Docker Website <https://www.docker.com>`_.

To pull the tutorial image, do:

.. code-block:: console

  $ docker pull laristra/flecsi-tutorial:latest

This will download the image to your machine. Once the pull is complete,
you can run the image like:

.. code-block:: console

  $ docker run -it -h tutorialhost -u flecsi laristra/flecsi-tutorial:latest /bin/bash

This will place you into a bash prompt, from which you can build and run
the tutorial examples. The tutorials are in the *flecsi-tutorial*
directory in the Docker container.

**Note:** You can also pull a Docker image for a specific runtime, e.g.,
mpi or legion. The default image *latest* is built against the MPI
backend. To pull the legion backend image do:

.. code-block:: console

  $ docker pull laristra/flecsi-tutorial:legion

Tutorial Examples
*****************

The tutorial examples are designed to guide the reader through basic to
more advanced FleCSI concepts. 

.. toctree::

  tutorial/flecsit
  tutorial/specialization
  tutorial/driver
  tutorial/tasks
  tutorial/index-spaces
  tutorial/fields
  tutorial/dense
  tutorial/sparse
  tutorial/ragged
  tutorial/mpi-interoperability

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
