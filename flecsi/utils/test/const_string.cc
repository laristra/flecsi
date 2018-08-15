/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/common.h>

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>

// =============================================================================
// Sanity check
// =============================================================================

// print_test
int32_t
print_test(flecsi::utils::const_string_t && s) {
  std::cout << "hash: " << s.hash() << std::endl;
  return 0;
} // print_test

// TEST
TEST(const_string, sanity) {
  print_test("hello world");
} // TEST

// =============================================================================
// More-complete exercising of const_string.h's constructs
// =============================================================================

// TEST
TEST(const_string, all) {

  // ------------------------
  // const_string_t
  // ------------------------

  using flecsi::utils::const_string_t;

  // hash_type_t
#ifdef __GNUG__
  EXPECT_EQ(flecsi::utils::type<typename const_string_t::hash_type_t>(), "unsigned long");
#endif

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
  } catch (...) {
    x = "caught an exception";
  }
  EXPECT_EQ(x, "caught an exception");

  // hash
  // Machine-dependent (via std::size_t); just do sanity checks
  EXPECT_TRUE(a.hash() > 0);
  const const_string_t b("");
  EXPECT_TRUE(b.hash() == 0); // known via i == n selection in hash__()
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

  const flecsi::utils::const_string_hasher_t hasher{};
  EXPECT_EQ(const_string_t("abc").hash(), hasher(const_string_t("abc")));

} // TEST

/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << std::endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
