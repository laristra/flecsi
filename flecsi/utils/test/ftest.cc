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

#include <flecsi/utils/ftest.hh>

int
init_a(int, char **) {
  flecsi::flog::devel_guard guard(flecsi::ftest_tag);
  flog(info) << "init a" << std::endl;

  flog(trace) << "trace (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(info) << "info (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(warn) << "warn (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  flog(error) << "error (strip level " << FLOG_STRIP_LEVEL << ")" << std::endl;
  return 0;
}

ftest_register_initialize(init_a);

int
init_b(int, char **) {
  flecsi::flog::devel_guard guard(flecsi::ftest_tag);
  flog(info) << "init b" << std::endl;
  return 0;
}

ftest_register_initialize(init_b);
ftest_add_initialize_dependency(init_b, init_a);

int
test1(int, char **) {

  FTEST();

  ASSERT_EQ(0, 0);
  EXPECT_EQ(0, 0);
  flog(info) << "result " << FTEST_RESULT() << std::endl;

  flecsi::flog::devel_guard guard(flecsi::ftest_tag);
  flog(info) << "THIS IS SOME LOG INFO FOR A TEST" << std::endl;
  return FTEST_RESULT();
}

ftest_register_driver(test1);

int
test2(int, char **) {

  FTEST();

  ASSERT_EQ(0, 0);
  int v{0};
  ASSERT_EQ(v, 0);
  return FTEST_RESULT();
}

ftest_register_driver(test2);

int
finalize(int, char **) {
  flecsi::flog::devel_guard guard(flecsi::ftest_tag);
  flog(info) << "finalize" << std::endl;
  return 0;
}

ftest_register_finalize(finalize);
