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
#include "flecsi/util/ftest.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topo;

using index_field_t = index_field_member<double>;
const index_field_t pressure_field;

const auto pressure = pressure_field(process_topology);

void
assign(index_field_t::accessor<wo> p) {
  flog(info) << "assign on " << color() << std::endl;
  p = color();
} // assign

int
check(index_field_t::accessor<ro> p) {
  FTEST {
    flog(info) << "check on " << color() << std::endl;
    ASSERT_EQ(p, color());
  };
} // print

int
index_driver(int, char **) {
  FTEST {
    execute<assign>(pressure);
    EXPECT_EQ(test<check>(pressure), 0);
  };
} // index

ftest_register_driver(index_driver);
