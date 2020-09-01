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

#include "4-state.hh"

#include "flecsi/flog.hh"
#include "flecsi/run/control.hh"

using namespace state;

int
allocate() {
  flog(info) << "allocate" << std::endl;

  /*
    Call a method of the control state to allocate an array of size 10.
   */

  control::state().allocate_values(10);

  return 0;
}
control::action<allocate, cp::allocate> allocate_action;

int
initialize() {
  flog(info) << "initialize" << std::endl;

  /*
    Access the array through the 'values()' method, and initialize.
   */

  size_t * const values = control::state().values();

  for(size_t i{0}; i < 10; ++i) {
    values[i] = 20 - i;
  } // for

  control::state().steps() = 5;

  return 0;
}
control::action<initialize, cp::initialize> initialize_action;

int
advance() {
  std::stringstream ss;

  ss << "advance " << control::state().step() << std::endl;

  /*
    Access the array through the 'values()' method, and modify.
   */

  size_t * const values = control::state().values();

  for(size_t i{0}; i < 10; ++i) {
    ss << values[i] << " ";
    values[i] = values[i] + 1;
  } // for

  ss << std::endl;

  flog(info) << ss.str();
  return 0;
}
control::action<advance, cp::advance> advance_action;

int
finalize() {
  flog(info) << "finalize" << std::endl;

  /*
    Deallocate the array using the control state interface.
   */

  control::state().deallocate_values();

  return 0;
}
control::action<finalize, cp::finalize> finalize_action;

int
main(int argc, char ** argv) {
  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::run::status::success) {
    return status == flecsi::run::status::help ? 0 : status;
  }

  status = control::check_options();

  if(status != flecsi::run::status::success) {
    flecsi::finalize();
    return status == flecsi::run::status::option ? 0 : status;
  } // if

  flecsi::log::add_output_stream("clog", std::clog, true);

  status = flecsi::start(control::execute);

  flecsi::finalize();

  return status;
} // main
