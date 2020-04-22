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

#include <flecsi/execution.hh>
#include <flecsi/util/unit.hh>

using namespace flecsi;

int
task_pass() {
  UNIT {
    flog(info) << "this test passes" << std::endl;
    ASSERT_EQ(1, 1);
  }; // UNIT
}

int
task_assert_fail() {
  UNIT {
    flog(info) << "this test fails an assertion" << std::endl;
    ASSERT_EQ(0, 1);
  }; // UNIT
}

int
task_expect_fail() {
  UNIT {
    flog(info) << "this test fails an expectation" << std::endl;
    EXPECT_EQ(0, 1);
  }; // UNIT
}

program_option<bool> fail("Test Options",
  "fail,f",
  "Force test failure.",
  {{flecsi::option_implicit, true}, {flecsi::option_zero}});

int
dag() {
  UNIT {
    ASSERT_EQ(test<task_pass>(), 0);
    ASSERT_NE(test<task_assert_fail>(), 0);
    ASSERT_NE(test<task_expect_fail>(), 0);

    flog(info) << "output from driver" << std::endl;

    EXPECT_EQ(test<task_pass>(), 0);
    EXPECT_NE(test<task_assert_fail>(), 0);
    EXPECT_NE(test<task_expect_fail>(), 0);

    // These show what happens during actual failure
    if(fail.has_value()) {
      EXPECT_EQ(test<task_expect_fail>(), 0);
      ASSERT_EQ(test<task_assert_fail>(), 0);
    } // if
  }; // UNIT
} // dag

flecsi::unit::driver<dag> driver;
