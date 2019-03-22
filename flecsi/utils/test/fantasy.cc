/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <flecsi/control/ftest.h>

int
init_a(int argc, char ** argv) {
  std::cout << "init a" << std::endl;
  return 0;
}

int
init_b(int argc, char ** argv) {
  std::cout << "init b" << std::endl;
  return 0;
}

ftest_register_initialize(init_a);
ftest_register_initialize(init_b);
ftest_add_initialize_dependency(init_b, init_a);

int
test1(int argc, char ** argv) {
  FTEST();
  ASSERT_EQ(0, 1);
  EXPECT_EQ(0, 1);
  return 0;
}

ftest_register_test(test1);

int
test2(int argc, char ** argv) {
  FTEST();
  ASSERT_EQ(0, 0);
  return 0;
}

ftest_register_test(test2);

int
finalize(int argc, char ** argv) {
  std::cout << "finalize" << std::endl;
  return 0;
}

ftest_register_finalize(finalize);
