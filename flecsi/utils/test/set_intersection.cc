/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include "flecsi/utils/set_intersection.h"

// includes: other
#include <cinchtest.h>

// intersects
template<class CONTAINER>
inline bool intersects(const CONTAINER &one, const CONTAINER &two)
{
   return
      flecsi::utils::intersects(
         one.begin(), one.end(),
         two.begin(), two.end()
      );
}



// =============================================================================
// Test flecsi::utils::set_intersection()
// =============================================================================

// TEST
TEST(set_intersection, all)
{
   std::vector<int> a = { 1, 3, 5, 7, 10, 11 };
   std::vector<int> b = { 2, 4, 6, 8, 10, 12 };
   std::vector<int> c = { 0, 20 };
   std::vector<int> d = { 10, 20, 30 };
   std::vector<int> e = { };
   std::vector<int> f = { };

   EXPECT_EQ(intersects(a, a), true );
   EXPECT_EQ(intersects(a, b), true );
   EXPECT_EQ(intersects(a, c), false);
   EXPECT_EQ(intersects(a, d), true );
   EXPECT_EQ(intersects(b, a), true );
   EXPECT_EQ(intersects(b, b), true );
   EXPECT_EQ(intersects(b, c), false);
   EXPECT_EQ(intersects(b, d), true );
   EXPECT_EQ(intersects(c, a), false);
   EXPECT_EQ(intersects(c, b), false);
   EXPECT_EQ(intersects(c, c), true );
   EXPECT_EQ(intersects(c, d), true );
   EXPECT_EQ(intersects(d, a), true );
   EXPECT_EQ(intersects(d, b), true );
   EXPECT_EQ(intersects(d, c), true );
   EXPECT_EQ(intersects(d, d), true );
   EXPECT_EQ(intersects(a, e), false);
   EXPECT_EQ(intersects(e, f), false);

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
