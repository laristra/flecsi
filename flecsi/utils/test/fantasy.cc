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

void init_a(int argc, char ** argv) {
  std::cout << "init a" << std::endl;
}

void init_b(int argc, char ** argv) {
  std::cout << "init b" << std::endl;
}

ftest_register_initialize(init_a);
ftest_register_initialize(init_b);
ftest_add_initialize_dependency(init_b, init_a);

void test1(int argc, char ** argv) {
  FTEST();
  ASSERT_EQ(0,1);
  EXPECT_EQ(0,1);
}

ftest_register_test(test1);

void test2(int argc, char ** argv) {
  FTEST();
  ASSERT_EQ(0,0);
}

ftest_register_test(test2);

void finalize(int argc, char ** argv) {
  std::cout << "finalize" << std::endl;
}

ftest_register_finalize(finalize);
