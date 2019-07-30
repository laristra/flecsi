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

template<size_t PRIVILEGES>
using dense_accessor =
  index_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

using index_field_t = index_field_member_u<double>;

const index_field_t nifld(1);
const auto nifh = nifld(flecsi_index_topology);

void
assign(dense_accessor<rw> ia) {
  flog(info) << "assign on " << color() << std::endl;
  ia = color();
} // assign

int
check(dense_accessor<ro> ia) {

  FTEST();

  flog(info) << "check on " << color() << std::endl;
  ASSERT_EQ(ia, color());

  return FTEST_RESULT();
} // print

int
index_driver(int argc, char ** argv) {

  execute<assign>(nifh);
  execute<check>(nifh);

  return 0;
} // index

ftest_register_driver(index_driver);
