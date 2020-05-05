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

#include "1-simple.hh"

#include "flecsi/execution.hh"
#include "flecsi/flog.hh"

using namespace simple;

/*
  Function definition of an initialize action.
 */

int
initialize() {
  flog(info) << "initialize" << std::endl;
  return 0;
}

/*
  Register the initialize action under the 'initialize' control point.
 */

control::action<initialize, cp::initialize> initialize_action;

/*
  Function definition of an advance action.
 */

int
advance() {
  flog(info) << "advance" << std::endl;
  return 0;
}

/*
  Register the advance action under the 'advance' control point.
 */

control::action<advance, cp::advance> advance_action;

/*
  Function definition of a finalize action.
 */

int
finalize() {
  flog(info) << "finalize" << std::endl;
  return 0;
}

/*
  Register the finalize action under the 'finalize' control point.
 */

control::action<finalize, cp::finalize> finalize_action;

/*
  The main function is similar to previous examples, but with the addition of
  logic to check control-model options.
 */

int
main(int argc, char ** argv) {
  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::run::status::success) {
    return status == flecsi::run::status::help ? 0 : status;
  }

  /*
    The check_options() method checks to see if any control-model options were
    specified on the command line, and handles them appropriately.
   */

  status = control::check_options();

  /*
    Check the return after control-model checks. Because this is after
    initialization, we need to call finalize if the program is exiting.
   */

  if(status != flecsi::run::status::success) {
    flecsi::finalize();
    return status == flecsi::run::status::control ? 0 : status;
  } // if

  flecsi::log::add_output_stream("clog", std::clog, true);

  /*
    Pass the control model's 'execute' method to start. FleCSI will invoke
    the execute function after runtime initialization. This will, in turn,
    execute all of the cycles, and actions of the control model.
   */

  status = flecsi::start(control::execute);

  flecsi::finalize();

  return status;
} // main
