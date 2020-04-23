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
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topo;

using double1 = field<double, singular>;
const double1::definition<global> energy_field;

const auto energy = energy_field(global_topology);

void
assign(double1::accessor<wo> ga) {
  flog(info) << "assign on " << color() << std::endl;
  ga = color();
} // assign

int
check(double1::accessor<ro> ga) {
  UNIT {
    static_assert(std::is_same_v<decltype(ga.get()), const double &>);
    flog(info) << "check on " << color() << std::endl;
    ASSERT_EQ(ga, 0);
  };
} // check

int
global_driver() {
  UNIT {
    execute<assign, single>(energy);
    EXPECT_EQ(test<check>(energy), 0);
  };
} // global_driver

flecsi::unit::driver<global_driver> driver;
