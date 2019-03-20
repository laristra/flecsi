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

#include <flecsi/utils/ftest.h>

void
init_a(int argc, char ** argv) {
  flog_tag_guard(ftest);
  flog(info) << "init a" << std::endl;

  flog(trace) << "trace (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(info) << "info (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(warn) << "warn (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(error) << "error (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
}

ftest_register_initialize(init_a);

void
init_b(int argc, char ** argv) {
  flog_tag_guard(ftest);
  flog(info) << "init b" << std::endl;
}

ftest_register_initialize(init_b);
ftest_add_initialize_dependency(init_b, init_a);

void
test1(int argc, char ** argv) {
  FTEST();
  ASSERT_EQ(0, 0);
  EXPECT_EQ(0, 0);

  flog_tag_guard(ftest);
  flog(info) << "THIS IS SOME LOG INFO FOR A TEST" << std::endl;
}

ftest_register_test(test1);

void
test2(int argc, char ** argv) {
  FTEST();
  ASSERT_EQ(0, 0);
}

ftest_register_test(test2);

void
finalize(int argc, char ** argv) {
  flog_tag_guard(ftest);
  flog(info) << "finalize" << std::endl;
}

ftest_register_finalize(finalize);
