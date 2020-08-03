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
  const auto i = process();
  flog(info) << "assign on " << i << std::endl;
  p = i;
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
    execute<assign, mpi>(pressure); // exercise MPI task with accessor
    EXPECT_EQ(test<check>(pressure), 0);
  };
} // index

flecsi::unit::driver<index_driver> driver;
