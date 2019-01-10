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
#include <flecsi/utils/utility.h>

void utility(int argc, char ** argv) {

  FTEST();

  using char_char_t = typename flecsi::utils::as_const<char, char>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(char_char_t) << std::endl;

  using char_int_t = typename flecsi::utils::as_const<char, int>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(char_int_t) << std::endl;

  using char_double_t =
    typename flecsi::utils::as_const<char, const volatile double>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(char_double_t) << std::endl;

#ifdef __GNUG__
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("utility.blessed.gnug"));
#endif
}

ftest_register_test(utility);
