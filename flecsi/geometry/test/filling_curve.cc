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

//! Basic tests for space filling curves, \TODO: Implement coordinates check

#include <cinchtest.h>

#include <flecsi/geometry/filling_curve.h>

using namespace flecsi;

using point_t = point_u<double, 3>;
using range_t = std::array<point_t, 2>;
using hc = hilbert_curve_u<3, uint64_t>;
using mc = morton_curve_u<3, uint64_t>;

TEST(filling_curve, hilbert) {
  range_t range;
  range[0] = {-1, -1, -1};
  range[1] = {1, 1, 1};
  point_t p1 = {0, 0, 0};

  std::cout << hc::max_depth() << std::endl;

  hc hc1;
  hc hc2(range, p1);
  hc hc3 = hc::min();
  hc hc4 = hc::max();
  hc hc5 = hc::root();
  std::cout << "Default: " << hc1 << std::endl;
  std::cout << "Pt:rg  : " << hc2 << std::endl;
  std::cout << "Min    : " << hc3 << std::endl;
  std::cout << "Max    : " << hc4 << std::endl;
  std::cout << "root   : " << hc5 << std::endl;
  ASSERT_TRUE(1 == hc5);

  while(hc4 != hc5) {
    hc4.pop();
  }
  ASSERT_TRUE(hc5 == hc4);
} // TEST

TEST(filling_curve, morton) {
  range_t range;
  range[0] = {-1, -1, -1};
  range[1] = {1, 1, 1};
  point_t p1 = {0, 0, 0};

  std::cout << hc::max_depth() << std::endl;

  mc hc1;
  mc hc2(range, p1);
  mc hc3 = mc::min();
  mc hc4 = mc::max();
  mc hc5 = mc::root();
  std::cout << "Default: " << hc1 << std::endl;
  std::cout << "Pt:rg  : " << hc2 << std::endl;
  std::cout << "Min    : " << hc3 << std::endl;
  std::cout << "Max    : " << hc4 << std::endl;
  std::cout << "root   : " << hc5 << std::endl;
  ASSERT_TRUE(1 == hc5);

  while(hc4 != hc5) {
    hc4.pop();
  }
  ASSERT_TRUE(hc5 == hc4);
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
