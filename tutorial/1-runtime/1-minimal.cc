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
#include <flecsi/utils/flog.hh>

/*
  The top-level action can be any C/C++ function that takes (int, char**) and
  returns an int.

  In this simple example, we only print a message to indicate that the
  top-level action was actually executed by FleCSI. However, in a real
  application, the top-level action would execute FleCSI tasks and other
  functions to implement the simulation.
 */

int top_level_action(int, char **) {
  std::cout << "Hello World" << std::endl;
  return 0;
} // top_level_action

/*
  This statement shows how to register a function as the top-level action that
  will be executed by FleCSI when start is invoked.
 */

inline bool top_level_action_registered =
  flecsi::register_top_level_action(top_level_action);

/*
  The main function must invoke initialize, start, and finalize on the FleCSI
  runtime. Otherwise, the implementation of main is left to the user.
 */

int main(int argc, char ** argv) {

  auto status = flecsi::initialize(argc, argv);

  /*
    The status returned by FleCSI's initialize method should be inspected to
    see if the end-user specified --help on the command line. FleCSI has
    built-in command-line support using Boost Program Options. This is
    documented in the next example.
   */

  if(status != flecsi::runtime::status::success) {
    return status == flecsi::runtime::status::help ? 0 : status;
  } // if

  status = flecsi::start();

  flecsi::finalize();

  return status;
} // main
