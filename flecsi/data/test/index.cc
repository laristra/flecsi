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
using namespace flecsi::data;
using namespace flecsi::topology;

using index_field_t = index_field_member_u<double>;
const index_field_t pressure_field;

const auto pressure = pressure_field(flecsi_index_topology);

void
assign(index_field_t::accessor<rw> p) {
  flog(info) << "assign on " << color() << std::endl;
  p = color();
} // assign

int
check(index_field_t::accessor<ro> p) {

  FTEST();

  flog(info) << "check on " << color() << std::endl;
  ASSERT_EQ(p, color());

  return FTEST_RESULT();
} // print

int
index_driver(int argc, char ** argv) {

  execute<assign>(pressure);
  execute<check>(pressure);

  return 0;
} // index

ftest_register_driver(index_driver);