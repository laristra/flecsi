/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/utility.h>
#include <flecsi/utils/common.h>

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>

// print_type
inline void
print_type(const char * const name) {
#ifdef __GNUG__
  CINCH_CAPTURE() << flecsi::utils::demangle(name) << std::endl;
#else
  // Skip name printing; is unpredictable in this case
#endif
}

// =============================================================================
// Test various constructs in utility.h
// =============================================================================

// TEST
TEST(utility, all) {
  print_type(typeid(typename flecsi::utils::as_const<char, char>::type).name());
  print_type(typeid(typename flecsi::utils::as_const<char, int>::type).name());
  print_type(
      typeid(
          typename flecsi::utils::as_const<char, const volatile double>::type)
          .name());

  // compare
#ifdef __GNUG__
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("utility.blessed.gnug"));
#endif
} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
