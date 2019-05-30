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
#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>
#include <flecsi/utils/demangle.h>
#include <flecsi/utils/ftest.h>

using namespace flecsi;

flecsi_add_global_field("test", "global", double, 2);
inline auto th = flecsi_global_field_instance("test", "global", double, 0);

template<size_t PRIVILEGES>
using global_accessor_u =
  flecsi::data::global_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

namespace global_test {

void
global_task(global_accessor_u<rw> ga, double value) {
  ga = value;
} // global_task

flecsi_register_task(global_task, global_test, loc, single);

void
print(global_accessor_u<ro> ga) {
  flog(info) << "Value: " << ga << std::endl;
} // print

flecsi_register_task(print, global_test, loc, single);

} // namespace global_test

int
global(int argc, char ** argv) {

  FTEST();

  double value{10.0};

  flecsi_execute_task(global_task, global_test, single, th, value);
  flecsi_execute_task(print, global_test, single, th);

  return 0;
}

ftest_register_driver(global);
