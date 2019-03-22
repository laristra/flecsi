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
#include <flecsi/utils/static_verify.h>

#include <iostream>

/*
  Some classes, with or without members foo and bar.
  These will facilitate our check of the FLECSI_MEMBER_CHECKER macro.
 */

struct first {
  int foo;
};

struct second {
  void bar(void) {}
};

struct both {
  int foo;
  void bar(void) {}
};

struct neither {};

// make sure two bars aren't counted as a foo and a bar
struct bars {
  void bar(void) {}
  void bar(int) {}
};

/*
  We'll be interested in checking classes for the presence or absence
  of members foo and bar. The following macro calls produce constructs
  that will facilitate our doing this.
 */
FLECSI_MEMBER_CHECKER(foo); // Makes has_member_foo<T>. ...Does T have foo?
FLECSI_MEMBER_CHECKER(bar); // Makes has_member_bar<T>. ...Does T have bar?

template<class T>
bool const has_member_foo<T>::value;
template<class T>
bool const has_member_bar<T>::value;

namespace flecsi {
namespace utils {

template<class T>
bool const is_tuple<T>::value;

template<class... T>
bool const is_tuple<std::tuple<T...>>::value;

} // namespace utils
} // namespace flecsi

int
static_verify(int argc, char ** argv) {

  FTEST();

  // first{} has foo only
  EXPECT_EQ(has_member_foo<first>::value, true);
  EXPECT_EQ(has_member_bar<first>::value, false);

  // second{} has bar only
  EXPECT_EQ(has_member_foo<second>::value, false);
  EXPECT_EQ(has_member_bar<second>::value, true);

  // both{} has both
  EXPECT_EQ(has_member_foo<both>::value, true);
  EXPECT_EQ(has_member_bar<both>::value, true);

  // neither{} has neither
  EXPECT_EQ(has_member_foo<neither>::value, false);
  EXPECT_EQ(has_member_bar<neither>::value, false);

  // bars{} has two bars, but no foo
  EXPECT_EQ(has_member_foo<bars>::value, false);
  EXPECT_EQ(has_member_bar<bars>::value, true);

  // ------------------------
  // is_tuple
  // ------------------------

  // with non-tuple
  EXPECT_EQ(flecsi::utils::is_tuple<int>::value, false);

  // with tuple
  EXPECT_EQ(flecsi::utils::is_tuple<std::tuple<>>::value, true);
  EXPECT_EQ(flecsi::utils::is_tuple<std::tuple<int>>::value, true);
  EXPECT_EQ((flecsi::utils::is_tuple<std::tuple<int, char>>::value), true);
  // the last line needed () because of ,

  return 0;
}

ftest_register_test(static_verify);
