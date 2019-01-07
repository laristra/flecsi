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

ftest_register_initializer(init_a);
ftest_register_initializer(init_b);
ftest_add_initializer_dependency(init_b, init_a);

void driver(int argc, char ** argv) {
  FTEST();

  ASSERT_EQ(0,0);
  
} // driver

ftest_register_driver(driver);
