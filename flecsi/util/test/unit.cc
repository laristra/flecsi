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

#include <flecsi/util/unit.hh>

using namespace flecsi;

int
init_a() {
  log::devel_guard guard(unit_tag);
  flog(info) << "init a" << std::endl;

  flog(trace) << "trace (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(info) << "info (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(warn) << "warn (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(error) << "error (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  return 0;
}

unit::initialization<init_a> ia_action;

int
init_b() {
  log::devel_guard guard(unit_tag);
  flog(info) << "init b" << std::endl;
  return 0;
}

unit::initialization<init_b> ib_action;
const auto ab = ib_action.add(ia_action);

int
test1() {
  UNIT {

    ASSERT_EQ(0, 0);
    EXPECT_EQ(0, 0);

    log::devel_guard guard(unit_tag);
    flog(info) << "THIS IS SOME LOG INFO FOR A TEST" << std::endl;
  };
}

unit::driver<test1> test1_driver;

int
test2() {
  UNIT {

    ASSERT_EQ(0, 0);
    int v{0};
    ASSERT_EQ(v, 0);
  };
}

unit::driver<test2> test2_driver;

int
finalization() {
  log::devel_guard guard(unit_tag);
  flog(info) << "finalize" << std::endl;
  return 0;
}

unit::finalization<finalization> f_action;
