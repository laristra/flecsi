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

// system includes
#include <cinchtest.h>

#include<vector>

// user includes
#include "flecsi/utils/reorder.h"

// some using declarations
using std::cout;
using std::endl;
using std::vector;

using flecsi::utils::reorder;
using flecsi::utils::reorder_destructive;

// the test data
const vector<int> v = { 0, 3, 2, 1 };
const vector<int> order = { 3, 0, 1, 2 };
const vector<int> ans = { 3, 2,  1, 0 };

//=============================================================================
//! \brief Test the inplace reordering
//=============================================================================
TEST(reorder, inplace) {

  auto vcpy = v;
  auto ocpy = order;
  reorder( ocpy.begin(), ocpy.end(), vcpy.begin() );
  ASSERT_EQ( vcpy, ans );


} // TEST


//=============================================================================
//! \brief Test the inplace reordering
//=============================================================================
TEST(reorder, destroy) {

  auto vcpy = v;
  auto ocpy = order;
  reorder_destructive( ocpy.begin(), ocpy.end(), vcpy.begin() );
  ASSERT_EQ( vcpy, ans );


} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
