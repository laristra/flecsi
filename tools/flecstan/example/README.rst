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

==================
flecstan: Examples
==================


Description
-----------

This directory contains 25 short example C++ codes that illustrate some
uses and misuses of a few of FleCSI's macros relating to tasks.

For each example C++ code, we have included a .json-format "compilation
database" file that illustrates how to communicate a build process to the
FleCSI Static Analyzer.

The .json files are, at present, specific to this author's computer, but
we believe you'll find them easy to adapt to your own environment. The
contents of the .json files are based on flags that are used by FleCSI's
"flecsit" python script that's used to compile FleCSI's tutorial codes.


Analysis
--------

We've included in this directory the output that the FleCSI analyzer
produced, for us, for each example C++ file. The analyzer was invoked simply
by providing one of the .json files as its argument; for example,

.. code-block:: console

     flecstan 01-task-good-register-execute-inside.json

Important: remember that you use the .json file here, not the .cc file! While
flecstan can accept C++ files directly, when doing so you must also supply
flecstan with knowledge of all the relevant compiler flags - essentially, the
contents of the .json file - in order for it to know what to do.


Colors
------

By default, the analyzer uses ANSI shell escape sequences to color its output.
Output is sent to C++ standard output; we've simply placed it into files here.

Our analysis files with the (default) color escape sequences have the .color
file extension. With luck, these will display with the intended colors, not with
strange characters, on your terminal. If you wish to try loading a .color file
into emacs for inspection, you may be able to place the following into
your .emacs file:

.. code-block:: console

     (require 'ansi-color)
     (defun color-text ()
       (interactive)
       (ansi-color-apply-on-region (point-min) (point-max)))

and, upon re-running emacs, use M-x color-text to convert color escape sequences
to actual colors. Other editors can probably be made to display colors properly,
if they don't do so automatically, by other means.

Files ending in .plain are identical to their .color counterparts, except that
they were produced with:

.. code-block:: console

     flecstan -no-color 01-task-good-register-execute-inside.json
     etc.

and thus don't contain the color escape sequences at all.


Naming Convention
-----------------

The C++ files in this directory have names as follows:

.. code-block:: console

     ##-task-CODE-DESCRIPTION.cc

The -task part is there to distinguish these examples from future,
non-task-related examples we may provide.

CODE is one of the following:

   - good
      - Code is good; no errors or warnings.

   - ugly
      - Good, but not particularly pretty.

   - warning
      - Has constructs that ideally should elicit warnings.

   - compile
      - Code has (a) compile-time error(s).
        Expect lots of output from flecstan in these cases!

   - run-flecsi
      - Code will compile, but has a problematic construct
        that FleCSI will in fact detect at run time.

   - run-segfault
      - Code will compile, but has a problematic construct
        that FleCSI will *not* detect at run time.
        So, you'd probably get a segfault if you compiled
        and ran one of these.

DESCRIPTION is a very brief description of what the code illustrates. (Very
brief, because it's part of the file name!) Each individual C++ file contains
comments that we hope will help you understand what the code is intended to
illustrate.
