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
#include "flecsi/util/demangle.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topo;

using double1 = field<double, singular>;
const double1::definition<topo::index> pressure_field;

const auto pressure = pressure_field(process_topology);

void
assign(double1::accessor<wo> p) {
  flog(info) << "assign on " << color() << std::endl;
  p = color();
} // assign

int
check(double1::accessor<ro> p) {
  UNIT {
    flog(info) << "check on " << color() << std::endl;
    ASSERT_EQ(p, color());
  };
} // print

int
index_driver() {
  UNIT {
    execute<assign>(pressure);
    EXPECT_EQ(test<check>(pressure), 0);
  };
} // index

flecsi::unit::driver<index_driver> driver;
