/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/utility.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/test/print_type.h>

// includes: C++
#include <iostream>

// includes: other
#include <ctest.h>

// =============================================================================
// Test various constructs in utility.h
// =============================================================================

// TEST
TEST(utility, all) {
  print_type<typename flecsi::utils::as_const<char, char>::type>();
  print_type<typename flecsi::utils::as_const<char, int >::type>();
  print_type<typename flecsi::utils::as_const<char, const volatile double>::type>();

  // compare
#ifdef __GNUG__
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("utility.blessed.gnug"));
#endif
} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
