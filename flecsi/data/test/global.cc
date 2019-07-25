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
#include <flecsi/data/data.hh>
#include <flecsi/execution/execution.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

namespace {
const data::field_interface_t::global_field<double> gfld(2);
const auto th = gfld(flecsi_global_topology);
} // namespace

template<size_t PRIVILEGES>
using global_accessor_u =
  flecsi::data::global_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

namespace global_test {

void
global_task(global_accessor_u<rw> ga, double value) {
  ga = value;
} // global_task

//flecsi_register_task(global_task, global_test, loc, single);

void
print(global_accessor_u<ro> ga) {
  flog(info) << "Value: " << ga << std::endl;
} // print

//flecsi_register_task(print, global_test, loc, single);

} // namespace global_test

int
global(int argc, char ** argv) {

  FTEST();

  double value{10.0};

//  flecsi_execute_task(global_task, global_test, single, th, value);
//  flecsi_execute_task(print, global_test, single, th);

  return FTEST_RESULT();
}

ftest_register_driver(global);
