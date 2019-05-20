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

flecsi_register_index_field("test", "index", double, 2);
inline auto th = flecsi_get_index_field("test", "index", double, 0);

template<size_t PRIVILEGES>
using index_accessor_u =
  flecsi::data::index_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

namespace index_test {

void
assignment(index_accessor_u<rw> ga, double value) {
  ga = value;
} // task

flecsi_register_task(assignment, index_test, loc, single);

void
print(index_accessor_u<ro> ga) {
  flog(info) << "Value: " << ga << std::endl;
} // print

flecsi_register_task(print, index_test, loc, single);

} // namespace index_test

int
test(int argc, char ** argv) {

  FTEST();

  double value{10.0};

  flecsi_execute_task(assignment, index_test, single, th, value);
  flecsi_execute_task(print, index_test, single, th);

  return 0;
}

ftest_register_test(test);
