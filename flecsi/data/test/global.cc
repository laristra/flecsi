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

#if 0
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

// flecsi_register_task(global_task, global_test, loc, single);

void
print(global_accessor_u<ro> ga) {
  flog(info) << "Value: " << ga << std::endl;
} // print

// flecsi_register_task(print, global_test, loc, single);

} // namespace global_test
#endif

#define __FLECSI_PRIVATE__
#include <flecsi/data/data.hh>
#include <flecsi/execution/execution.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topology;

template<size_t PRIVILEGES>
using global_accessor = global_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

using global_field_t = global_field_member_u<double>;

const global_field_t energy_field(1);
const auto energy = energy_field(flecsi_global_topology);

void
assign(global_accessor<rw> ga) {
  flog(info) << "assign on " << color() << std::endl;
  ga = color();
} // assign

int
check(global_accessor<ro> ga) {
  FTEST();

  flog(info) << "check on " << color() << std::endl;
  ASSERT_EQ(ga, 0);

  return FTEST_RESULT();
} // check

int
global(int argc, char ** argv) {

  double value{10.0};

  execute<assign>(energy);
  execute<check>(energy);

  return 0;
}

ftest_register_driver(global);
