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

#define __FLECSI_PRIVATE__
#include <flecsi/execution/execution.hh>
#include <flecsi/io/io_interface.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

namespace io_test {

void
check() {

  FTEST();

} // check

flecsi_register_task(check, io_test, loc, index);

} // namespace io_test

int
io_sanity(int argc, char ** argv) {

  flecsi_execute_task(check, io_test, index);

  return 0;
} // io_sanity

ftest_register_driver(io_sanity);
