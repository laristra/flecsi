/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#include <cinchtest.h>

#include "flecsi/geometry/point.h"

using namespace flecsi;

using point_1d_t = point<double,1>;
using point_2d_t = point<double,2>;
using point_3d_t = point<double,3>;

TEST(point, sanity) {
  point_1d_t a1{-1.0};
  ASSERT_EQ(-1.0, a1[utils::axis::x]);

  point_2d_t a2{3.0, 0.0};
  ASSERT_EQ(3.0, a2[utils::axis::x]);
  ASSERT_EQ(0.0, a2[utils::axis::y]);

  point_3d_t a3{3.0, 0.0, -1.0};
  ASSERT_EQ(3.0, a3[utils::axis::x]);
  ASSERT_EQ(0.0, a3[utils::axis::y]);
  ASSERT_EQ(-1.0, a3[utils::axis::z]);

} // TEST

TEST(point, distance) {
  point_1d_t a1{1.0};
  point_1d_t b1{4.0};
  double d = distance(a1, b1);
  ASSERT_EQ(3.0, d) << "Distance calculation failed";

  point_2d_t a2{1.0, 2.0};
  point_2d_t b2{4.0, 6.0};
  d = distance(a2, b2);
  ASSERT_EQ(5.0, d) << "Distance calculation failed";

  point_3d_t a3{1.0, 2.0, -1.0};
  point_3d_t b3{4.0, 6.0, -1.0 - std::sqrt(11.0)};
  d = distance(a3, b3);
  ASSERT_EQ(6.0, d) << "Distance calculation failed";
} // TEST

TEST(point, midpoint) {
  point_1d_t a1{1.0};
  point_1d_t b1{4.0};
  point_1d_t c1 = midpoint(a1, b1);
  ASSERT_EQ(2.5, c1[0]) << "Midpoint calculation failed";

  point_2d_t a2{1.0, 2.0};
  point_2d_t b2{4.0, 6.0};
  point_2d_t c2 = midpoint(a2, b2);
  ASSERT_EQ(2.5, c2[0]) << "Midpoint calculation failed";
  ASSERT_EQ(4.0, c2[1]) << "Midpoint calculation failed";

  point_3d_t a3{1.0, 2.0, -1.0};
  point_3d_t b3{4.0, 6.0, -4.0};
  point_3d_t c3 = midpoint(a3, b3);
  ASSERT_EQ(2.5, c3[0]) << "Midpoint calculation failed";
  ASSERT_EQ(4.0, c3[1]) << "Midpoint calculation failed";
  ASSERT_EQ(-2.5, c3[2]) << "Midpoint calculation failed";
} // TEST


/*----------------------------------------------------------------------------*
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
 *
 *----------------------------------------------------------------------------*/

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
