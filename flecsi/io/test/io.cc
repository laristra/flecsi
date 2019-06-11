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
#include <flecsi/execution/execution.h>
#include <flecsi/io/io_interface.h>
#include <flecsi/utils/ftest.h>

using namespace flecsi;

namespace io_test {

void
check() {

  FTEST();

} // check

flecsi_register_task(check, io_test, loc, index);

} // namespace io_test

int
io_sanity(int argc, char ** argv) {} // io_sanity

ftest_register_driver(io_sanity);
