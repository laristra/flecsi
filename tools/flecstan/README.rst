.. |br| raw:: html

..
   -----------------------------------------------------------------------------
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2019, Triad National Security, LLC
   All rights reserved.
   -----------------------------------------------------------------------------

<br />

flecstan: FleCSI Static Analyzer
================================

Introduction
------------

This is an initial, simple "readme" for the very first release of flecstan.


Compilation
-----------

There are two ways to compile flecstan:

With CMake:

.. code-block:: console

   $ mkdir -p build
   $ cd build
   $ cmake ..
   $ make

With our bash script:

.. code-block:: console

   $ ./compile.sh

flecstan uses the clang/llvm tooling libraries, so those will need to be
available on your machine.


Notes
-----

It seems that clang/llvm-based tools don't necessarily behave correctly unless
they're placed into the directory where clang++ resides; say, /usr/local/bin/.
See here:

`http://lists.llvm.org/pipermail/cfe-dev/2014-March/035806.html
<http://lists.llvm.org/pipermail/cfe-dev/2014-March/035806.html>`_.

for more information. For now, then, whether you compile with CMake or with the
bash script, you might consider moving the executable (flecstan) to the location
in question before running it. We'd have the cmake and bash-script processes do
this themselves, except that the move might require super-user permissions.

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
