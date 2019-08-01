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

#include <flecsi/utils/ftest.hh>

#define __FLECSI_PRIVATE__
#include <flecsi/execution/execution.hh>

flog_register_tag(task);

using namespace flecsi;

namespace hydro {

template<typename TYPE>
void
simple(TYPE arg) {
  flog_tag_guard(task);
  flog(info) << "arg(" << arg << ")\n";
} // simple

} // namespace hydro

int
test_driver(int argc, char ** argv) {

  FTEST();

  execute<hydro::simple<float>>(6.2);

  return FTEST_RESULT();
}

ftest_register_driver(test_driver);
