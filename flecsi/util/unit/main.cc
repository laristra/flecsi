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
#include <flecsi/util/unit.hh>

int
main(int argc, char ** argv) {

  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::run::status::success) {
    return status == flecsi::run::status::help ? 0 : status;
  } // if

  status = flecsi::unit::control::check_options();

  if(status != flecsi::run::status::success) {
    flecsi::finalize();
    return status == flecsi::run::status::option ? 0 : status;
  } // if

  status = flecsi::start(flecsi::unit::control::execute);

  flecsi::finalize();

  return status;
} // main
