/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include "flecsi/utils/utility.h"

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>
#include "boost/core/demangle.hpp"

// print_type
inline void print_type(const char *const name)
{
   CINCH_CAPTURE() << boost::core::demangle(name) << std::endl;
}



// =============================================================================
// Test various constructs in utility.h
// =============================================================================

// TEST
TEST(utility, all)
{
   print_type(typeid(typename
      flecsi::utils::as_const<char, char>::type).name());
   print_type(typeid(typename
      flecsi::utils::as_const<char, int>::type).name());
   print_type(typeid(typename
      flecsi::utils::as_const<char, const volatile double>::type).name());

   // compare
   EXPECT_TRUE(CINCH_EQUAL_BLESSED("utility.blessed"));

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
