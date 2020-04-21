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

#include "flecsi/util/const_string.hh"
#include "flecsi/util/common.hh"
#include "flecsi/util/ftest.hh"

int
const_string(int, char **) {
  FTEST {
    // ------------------------
    // const_string_t
    // ------------------------

    using flecsi::util::const_string_t;

    // constructor from C-style string
    // ...................0123456789
    const char str[10] = "abcdefghi";
    const_string_t a(str);

    // c_str, size
    EXPECT_EQ(std::string(a.c_str()), "abcdefghi");
    EXPECT_EQ(a.size(), 9);

    // operator[]
    EXPECT_EQ(a[0], 'a');
    EXPECT_EQ(a[1], 'b');
    EXPECT_EQ(a[2], 'c');
    EXPECT_EQ(a[8], 'i');
    std::string x = "no exception";
    try {
      (void)a[9]; // considered to be out-of-range (not the \0)
    }
    catch(...) {
      x = "caught an exception";
    }
    EXPECT_EQ(x, "caught an exception");

    // hash
    // Machine-dependent (via std::size_t); just do sanity checks
    EXPECT_TRUE(a.hash() > 0);
    const const_string_t b("");
    EXPECT_TRUE(b.hash() == 0);
    const const_string_t c("x");
    EXPECT_TRUE(c.hash() > 0);

    // ==, !=
    EXPECT_TRUE(const_string_t("") == const_string_t(""));
    EXPECT_TRUE(const_string_t("") != const_string_t("a"));
    EXPECT_TRUE(const_string_t("a") != const_string_t(""));
    EXPECT_TRUE(const_string_t("abc") == const_string_t("abc"));
    EXPECT_TRUE(const_string_t("abc") != const_string_t("abcd"));
    EXPECT_TRUE(const_string_t("abc") != const_string_t("dabc"));

    // ------------------------
    // const_string_hasher_t
    // ------------------------

    const flecsi::util::const_string_hasher_t hasher{};
    EXPECT_EQ(const_string_t("abc").hash(), hasher(const_string_t("abc")));
  };
}

ftest_register_driver(const_string);
