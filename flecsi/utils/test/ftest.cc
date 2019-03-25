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

int
init_a(int argc, char ** argv) {
  flog_tag_guard(ftest);
  flog(info) << "init a" << std::endl;

  flog(trace) << "trace (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(info) << "info (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(warn) << "warn (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(error) << "error (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  return 0;
}

ftest_register_initialize(init_a);

int
init_b(int argc, char ** argv) {
  flog_tag_guard(ftest);
  flog(info) << "init b" << std::endl;
  return 0;
}

ftest_register_initialize(init_b);
ftest_add_initialize_dependency(init_b, init_a);

int
test1(int argc, char ** argv) {
  FTEST();
  ASSERT_EQ(0, 0);
  EXPECT_EQ(0, 1);
  EXPECT_EQ(0, 0);
  flog(info) << "result " << FTEST_RESULT() << std::endl;

  flog_tag_guard(ftest);
  flog(info) << "THIS IS SOME LOG INFO FOR A TEST" << std::endl;
  return FTEST_RESULT();
}

ftest_register_test(test1);

int
test2(int argc, char ** argv) {
  FTEST();
  ASSERT_EQ(0, 0);
  int v{0};
  ASSERT_EQ(v, 1);
  std::cerr << "result " << FTEST_RESULT() << std::endl;
  return FTEST_RESULT();
}

ftest_register_test(test2);

int
finalize(int argc, char ** argv) {
  flog_tag_guard(ftest);
  flog(info) << "finalize" << std::endl;
  return 0;
}

ftest_register_finalize(finalize);
