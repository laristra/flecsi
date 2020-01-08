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

int
main(int argc, char ** argv) {

  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::runtime::status::success) {
    return status == flecsi::runtime::status::help ? 0 : status;
  } // if

  status = flecsi::start();

  flecsi::finalize();

  return status;
} // main
