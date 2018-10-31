Example 1: Driver
=================

As a runtime abstraction layer, FleCSI replaces the normal main
function with a driver function. Like main, the driver is the most
fundamental unit of execution, providing the top-level control logic
for the user's code. There is still a main function as required by
C/C++. However, in a FleCSI application, it is normally handled by
the specialization layer, or, in the case of this tutorial, by the
flecsit compiler script, which uses the runtime_main.cc file that is
installed as part of FleCSI under the *share/FleCSI/runtime* directory.
Applications can choose to either use this default main function, or
replace it with their own (if more explicit control is needed).

This example's driver prints out the canonical *Hello World* with
whatever arguments the user has passed to the command line.
Command-line arguments are passed through the FleCSI runtime to the
user's driver as with a normal main function.

Try compiling this example with flecsit:

.. code-block:: console

  $ flecsit compile driver.cc

You can run it like:

.. code-block:: console

  $ ./driver arg1 arg2

You can experiment with changing the output or handling of the
command-line arguments.

NOTES:

* All drivers must be defined in the flecsi::execution namespace.

The code for this example can be found in *driver.cc*:

.. code-block:: cpp

  #include <iostream>

  namespace flecsi {
  namespace execution {

  void driver(int argc, char ** argv) {

    // Print the message

    std::cout << "Hello World" << std::endl;

    // Print the arguments that were passed on the command line

    for(size_t i{1}; i<argc; ++i) {
      std::cout << "\targ(" << i << "): " << argv[i] << std::endl;
    } // for

  } // driver

  } // namespace execution
  } // namespace flecsi

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
