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
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

int
task_pass() {
  FTEST();
  flog(info) << "this test passes" << std::endl;
  ASSERT_EQ(1, 1);
  return FTEST_RESULT();
}

int
task_assert_fail() {
  FTEST();
  flog(info) << "this test fails an assertion" << std::endl;
  ASSERT_EQ(0, 1);
  return FTEST_RESULT();
}

int
task_expect_fail() {
  FTEST();
  flog(info) << "this test fails an expectation" << std::endl;
  EXPECT_EQ(0, 1);
  return FTEST_RESULT();
}

program_option<bool> fail("Test Options",
  "fail,f",
  "Force test failure.",
  {{flecsi::option_implicit, true}, {flecsi::option_zero}});

int
driver(int, char **) {
  FTEST();

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

  return FTEST_RESULT();
} // driver

ftest_register_driver(driver);
