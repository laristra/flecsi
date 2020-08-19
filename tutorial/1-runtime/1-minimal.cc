/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#include <flecsi/execution.hh>

/*
  The top-level action can be any C/C++ function that takes no arguments and
  returns an int.

  In this simple example, we only print a message to indicate that the
  top-level action was actually executed by FleCSI. However, in a real
  application, the top-level action would execute FleCSI tasks and other
  functions to implement the simulation.
 */

int
top_level_action() {
  std::cout << "Hello World" << std::endl;
  return 0;
} // top_level_action

/*
  The main function must invoke initialize, start, and finalize on the FleCSI
  runtime. Otherwise, the implementation of main is left to the user.
 */

int
main(int argc, char ** argv) {

  auto status = flecsi::initialize(argc, argv);

  /*
    The status returned by FleCSI's initialize method should be inspected to
    see if the end-user specified --help on the command line. FleCSI has
    built-in command-line support using Boost Program Options. This is
    documented in the next example.
   */

  if(status != flecsi::run::status::success) {
    return status == flecsi::run::status::help ? 0 : status;
  } // if

  /*
    The top-level action is passed to the start function to tell FleCSI what
    it should execute.
   */

  status = flecsi::start(top_level_action);

  flecsi::finalize();

  return status;
} // main
