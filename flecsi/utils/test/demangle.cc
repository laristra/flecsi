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
demangle(int, char **) {

  FTEST();

  // demangle, type
  // The results depend on #ifdef __GNUG__, so we'll just exercise
  // these functions, without checking for particular results.
  EXPECT_NE(flecsi::utils::demangle("foo"), "");

  auto str_demangle = FTEST_TTYPE(int);
  auto str_type = flecsi::utils::type<int>();

  EXPECT_NE(str_demangle, "");
  EXPECT_NE(str_type, "");
  EXPECT_EQ(str_demangle, str_type);

  const auto sym = flecsi::utils::symbol<demangle>();
#ifdef __GNUG__
  EXPECT_EQ(sym, "demangle(int, char**)");
#else
  EXPECT_NE(sym, "");
#endif

  return 0;
}

ftest_register_driver(demangle);
