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

#include "flecsi/geometry/space_vector.h"

using namespace flecsi;

using vector_1d_t = space_vector<double,1>;
using vector_2d_t = space_vector<double,2>;
using vector_3d_t = space_vector<double,3>;

TEST(space_vector, plus) {
  vector_1d_t a1(1.0), b1(2.0);
  vector_1d_t c1 = a1 + b1;
  ASSERT_EQ(3.0, c1[0]);

  vector_2d_t a2(1.0,1.0), b2(2.0,2.0);
  vector_2d_t c2 = a2 + b2;
  ASSERT_EQ(3.0, c2[0]);
  ASSERT_EQ(3.0, c2[1]);

  vector_3d_t a3(1.0,1.0,1.0), b3(2.0,2.0,2.0);
  vector_3d_t c3 = a3 + b3;
  ASSERT_EQ(3.0, c3[0]);
  ASSERT_EQ(3.0, c3[1]);
  ASSERT_EQ(3.0, c3[2]);
}

TEST(space_vector, plusequal) {
  vector_1d_t a1(1.0), b1(2.0);
  a1 += b1;
  ASSERT_EQ(3.0, a1[0]);

  vector_2d_t a2(1.0,1.0), b2(2.0,2.0);
  a2 += b2;
  ASSERT_EQ(3.0, a2[0]);
  ASSERT_EQ(3.0, a2[1]);

  vector_3d_t a3(1.0,1.0,1.0), b3(2.0,2.0,2.0);
  a3 += b3;
  ASSERT_EQ(3.0, a3[0]);
  ASSERT_EQ(3.0, a3[1]);
  ASSERT_EQ(3.0, a3[2]);
}

TEST(space_vector, minus) {
  vector_1d_t a1(1.0), b1(2.0);
  vector_1d_t c1 = a1 - b1;
  ASSERT_EQ(-1.0, c1[0]);

  vector_2d_t a2(1.0,1.0), b2(2.0,2.0);
  vector_2d_t c2 = a2 - b2;
  ASSERT_EQ(-1.0, c2[0]);
  ASSERT_EQ(-1.0, c2[1]);

  vector_3d_t a3(1.0,1.0,1.0), b3(2.0,2.0,2.0);
  vector_3d_t c3 = a3 - b3;
  ASSERT_EQ(-1.0, c3[0]);
  ASSERT_EQ(-1.0, c3[1]);
  ASSERT_EQ(-1.0, c3[2]);
}

TEST(space_vector, minusequal) {
  vector_1d_t a1(1.0), b1(2.0);
  a1 -= b1;
  ASSERT_EQ(-1.0, a1[0]);

  vector_2d_t a2(1.0,1.0), b2(2.0,2.0);
  a2 -= b2;
  ASSERT_EQ(-1.0, a2[0]);
  ASSERT_EQ(-1.0, a2[1]);

  vector_3d_t a3(1.0,1.0,1.0), b3(2.0,2.0,2.0);
  a3 -= b3;
  ASSERT_EQ(-1.0, a3[0]);
  ASSERT_EQ(-1.0, a3[1]);
  ASSERT_EQ(-1.0, a3[2]);
}

TEST(space_vector, operator_times) {
  double s = 2.0;

  // 1d scalar vector multiply
  vector_1d_t a(4.0);
  vector_1d_t as = a*s;
  ASSERT_EQ(8.0, as[0]);

  // 2d scalar vector multiply
  vector_2d_t b(4.0, -5.0);
  vector_2d_t bs = b*s;
  ASSERT_EQ(  8.0, bs[0]);
  ASSERT_EQ(-10.0, bs[1]);

  // 3d scalar vector multiply
  vector_3d_t c(4.0, -5.0, 6.0);
  vector_3d_t cs = c*s;
  ASSERT_EQ(  8.0, cs[0]);
  ASSERT_EQ(-10.0, cs[1]);
  ASSERT_EQ( 12.0, cs[2]);

} // TEST

TEST(space_vector, dot) {
  // 1d vector dot
  vector_1d_t a1(1.0);
  vector_1d_t b1(3.0);
  ASSERT_EQ(3.0, dot(a1, b1));

  a1[0] = -1.0;
  ASSERT_EQ(-3.0, dot(a1, b1));

  // 2d vector dot
  vector_2d_t a2(1.0, 1.0);
  vector_2d_t b2(3.0, 4.0);
  ASSERT_EQ(7.0, dot(a2, b2));

  a2[1] = -1.0;
  ASSERT_EQ(-1.0, dot(a2, b2));

  // 3d vector dot
  vector_3d_t a3(1.0, 1.0, 1.0);
  vector_3d_t b3(3.0, 4.0, 5.0);
  ASSERT_EQ(12.0, dot(a3, b3));

  b3[2] = -5.0;
  ASSERT_EQ(2.0, dot(a3, b3));

} // TEST

TEST(space_vector, magnitude) {

  vector_1d_t a(7.0);
  ASSERT_EQ(7.0, magnitude(a));

  a[0] = -5.0;
  ASSERT_EQ(5.0, magnitude(a));

  vector_2d_t b(3.0, 4.0);
  ASSERT_EQ(5.0, magnitude(b));

  b[1] = -4.0;
  ASSERT_EQ(5.0, magnitude(b));

  vector_3d_t c(3.0, 4.0, 5.0);
  ASSERT_EQ(sqrt(50.0), magnitude(c));

  c[2] = -5.0;
  ASSERT_EQ(sqrt(50.0), magnitude(c));

} // TEST

TEST(space_vector, cross_magnitude) {
  vector_1d_t a1(1.0);
  vector_1d_t b1(2.0);
  double c1 = cross_magnitude(a1, b1);
  ASSERT_EQ(0.0, c1);

  vector_2d_t a2(1.0, 3.0);
  vector_2d_t b2(2.0,-5.0);
  double c2 = cross_magnitude(a2, b2);
  ASSERT_EQ(11.0, c2);

  vector_3d_t a3(1.0, 3.0, -2.0);
  vector_3d_t b3(2.0,-5.0, 10.0);
  double c3 = cross_magnitude(a3, b3);
  ASSERT_EQ(sqrt(20.0*20.0 + 14.0*14.0 + 11.0*11.0), c3);

} // TEST

using point_2d_t = point<double,2>;
using point_3d_t = point<double,3>;

TEST(space_vector, normal) {

  // 1d vector normal not defined.

  // 2d normal
  point_2d_t a2(1.0, 1.0);
  point_2d_t b2(3.0, 4.0);

  vector_2d_t v2 = normal(a2,b2);
  ASSERT_EQ(-3.0, v2[0]);
  ASSERT_EQ( 2.0, v2[1]);

  // 3d normal
  vector_3d_t a3(1.0, 1.0, 1.0);
  vector_3d_t b3(3.0, 4.0, 5.0);

  vector_3d_t v3 = normal(a3, b3);


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
