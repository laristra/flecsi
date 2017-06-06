/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include "flecsi/utils/hash.h"

// includes: other
#include <cinchtest.h>



// =============================================================================
// Test various aspects of flecsi::utils::hash
// =============================================================================

TEST(hash, all)
{
   // ------------------------
   // test: hash__()
   // ------------------------

   // the third argument == the second argument; return the first argument
   EXPECT_EQ(flecsi::utils::hash__("abc", 123UL, 0, 0), 123);
   EXPECT_EQ(flecsi::utils::hash__("abc", 456UL, 1, 1), 456);
   EXPECT_EQ(flecsi::utils::hash__("abc", 789UL, 2, 2), 789);

   // the third argument < the second argument; return something more complex
   EXPECT_EQ(flecsi::utils::hash__("abc", 123, 0, 1),      26);
   EXPECT_EQ(flecsi::utils::hash__("abc", 456, 1, 2),   25544);
   EXPECT_EQ(flecsi::utils::hash__("abc", 789, 1, 3), 6512917);

   // no, because 4 is greater than the string's length
   // EXPECT_EQ(flecsi::utils::hash__("abc", 123, 0, 4), );

   // no, because we'd keep ++'ing 1 until we get 0 :-/
   // EXPECT_EQ(flecsi::utils::hash__("abc", 123, 1, 0), );

   // ------------------------
   // test: hash()
   // ------------------------

   EXPECT_EQ(flecsi::utils::hash<std::size_t>("",      0),            0);
   EXPECT_EQ(flecsi::utils::hash<std::size_t>("1",     1),           49);
   EXPECT_EQ(flecsi::utils::hash<std::size_t>("12",    2),        12849);
   EXPECT_EQ(flecsi::utils::hash<std::size_t>("123",   3),      3355185);
   EXPECT_EQ(flecsi::utils::hash<std::size_t>("1234",  4),    875770417);
   EXPECT_EQ(flecsi::utils::hash<std::size_t>("12345", 5), 228509037105);

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
