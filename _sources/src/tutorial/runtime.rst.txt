.. |br| raw:: html

   <br />

Runtime
=======

Using FleCSI requires proper initialization and configuration of the
FleCSI runtime. These examples illustrate some of the basic steps and
options that are available.

Example 1: Minimal
******************

The core FleCSI runtime has three control points: *initialize*, *start*, and
*finalize*. These must be invoked by the user's application code in that
order.

.. sidebar:: Top-level Action

  The top-level action is a C++ function created by the user to tell
  FleCSI what it should do when the start control point is invoked.

* **initialize** |br|
  This control point spins up the FleCSI runtime, and (optionally) any
  other runtimes on which FleCSI depends.

* **start** |br|
  This control point begins the actual runtime execution that forms the
  bulk of any simulation that is performed using FleCSI. The user must
  pass a top-level action that FleCSI will execute.

* **finalize** |br|
  This control point shuts down the FleCSI runtime, and any other
  runtimes that FleCSI itself initialized.

This example demonstrates a minimal use of FleCSI that just executes an
action to print out *Hello World*. Code for this example can be found in
*tutorial/1-runtime/1-minimal.cc*

.. literalinclude:: ../../../tutorial/1-runtime/1-minimal.cc
  :language: cpp

.. note::

  - The top-level action can be any C/C++ function that takes (int,
    char**) and returns an int.  In this simple example, we only print a
    message to indicate that the top-level action was actually executed
    by FleCSI. However, in a real application, the top-level action
    would execute FleCSI tasks and other functions to implement the
    simulation.  

  - The main function must invoke initialize, start, and finalize on the
    FleCSI runtime. Otherwise, the implementation of main is left to the
    user.

  - The status returned by FleCSI's initialize method should be
    inspected to see if the end-user specified --help on the command
    line. FleCSI has built-in command-line support using Boost Program
    Options. This is documented in the next example.

Example 2: Program Options
**************************

FleCSI supports a program options capability based on `Boost Program
Options`__ to simplify the creation and management of user-defined
command-line options. The basic syntax for adding and accessing program
options is similar to the Boost interface (You can find documentation
using the above link.)  However, FleCSI makes it easier for applications
to add options by removing restrictions on where they may be defined.

__ https://www.boost.org/doc/libs/1_63_0/doc/html/program_options.html

This example extends the previous example to add some command-line
options.

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp

Example 3: FLOG (FleCSI Logging Utility)
****************************************

.. literalinclude:: ../../../tutorial/1-runtime/3-flog.cc
  :language: cpp

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
