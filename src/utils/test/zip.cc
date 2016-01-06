/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

// system includes
#include <cinchtest.h>

#include<array>
#include<iostream>
#include<list>
#include<typeinfo>
#include<vector>

// user includes
#include "flexi/utils/zip.h"

// some using declarations
using std::array;
using std::cout;
using std::endl;
using std::list;
using std::vector;

using flexi::utils::get;
using flexi::utils::zip;

//=============================================================================
//! \brief Test the "zip-like" iterator.
//=============================================================================
TEST(zip, simple) {

  vector<double> a{1.0, 2.0, 3.0, 4.0};
  list<char> b;
  b.push_back('a');
  b.push_back('b');
  b.push_back('c');
  b.push_back('d');
  array<int,4> c{4,3,2,1};
  array<int,5> c_fail{4,3,2,1};

  // should die
  ASSERT_DEATH( zip(a, b, c_fail), "size mismatch" );

  // now test for reals
  auto d = zip(a, b, c);

  for (auto i : zip(a, b, c) ) {
    cout << get<0>(i) << ", " << get<1>(i) << ", " << get<2>(i) << endl;
  }

  for (auto i : d) {
    cout << get<0>(i) << ", " << get<1>(i) << ", " << get<2>(i) << endl;
    get<0>(i) = 5;
    //cout << i1 << ", " << i2 << ", " << i3 << endl;
  }
  for (const auto i : d) {
    ASSERT_EQ( 5.0, get<0>(i) );
    cout << get<0>(i) << ", " << get<1>(i) << ", " << get<2>(i) << endl;
    //cout << i1 << ", " << i2 << ", " << i3 << endl;
  }  

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
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << endl;
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

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
