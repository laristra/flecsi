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

-----------
Description
-----------

This directory contains five short C++ source examples of certain mistakes
that could be made in the use of FleCSI constructs:

   - Incorrect data model
   - Incorrect type
   - Mismatching name
   - Mismatching namespace
   - Version out of range

See each ``.cc`` file for specific remarks.

-----------
Input
-----------

The ``.json`` files show how one might construct a ``.json`` "compilation
database" that informs Flecsi Static Analyzer how to compile the codes.

Note that each of these examples requires the FleCSI tutorial specialization
codes, as in the ``flecsi/flecsi-tutorial/`` directory.

-----------
Output
-----------

Sample output files come from running the Flecsi Static Analyzer in a few
different ways:

   - ``*.full``   With no particular command-line options
   - ``*.color``  With ``--quiet`` mode
   - ``*.plain``  With ``--quiet`` mode and ``--no-color``
