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

#include "flecsi/ctrl/ftest.hh"

int
init_a(int, char **) {
  std::cout << "init a" << std::endl;
  return 0;
}

int
init_b(int, char **) {
  std::cout << "init b" << std::endl;
  return 0;
}

ftest_register_initialize(init_a);
ftest_register_initialize(init_b);
ftest_add_initialize_dependency(init_b, init_a);

int
test1(int, char **) {
  FTEST {
    ASSERT_EQ(0, 1);
    EXPECT_EQ(0, 1);
  };
}

ftest_register_driver(test1);

int
test2(int, char **) {
  FTEST { ASSERT_EQ(0, 0); };
}

ftest_register_driver(test2);

int
finalize(int, char **) {
  std::cout << "finalize" << std::endl;
  return 0;
}

ftest_register_finalize(finalize);
