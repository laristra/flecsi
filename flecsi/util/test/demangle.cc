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

#include "flecsi/util/unit.hh"

int
demangle() {
  UNIT {
    // demangle, type
    // The results depend on #ifdef __GNUG__, so we'll just exercise
    // these functions, without checking for particular results.
    EXPECT_NE(flecsi::util::demangle("foo"), "");

    auto str_demangle = UNIT_TTYPE(int);
    auto str_type = flecsi::util::type<int>();

    EXPECT_NE(str_demangle, "");
    EXPECT_NE(str_type, "");
    EXPECT_EQ(str_demangle, str_type);

    const auto sym = flecsi::util::symbol<demangle>();
#ifdef __GNUG__
    EXPECT_EQ(sym, "demangle()");
#else
    EXPECT_NE(sym, "");
#endif
  };
} // demangle

flecsi::unit::driver<demangle> driver;
