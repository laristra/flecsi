/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/simple_id.h>

// includes: other
#include <cinchtest.h>

using namespace flecsi::utils;

// TEST
TEST(simple_id, all) {

  auto a = make_simple_id( 1, 2, 3 );
  auto b = make_simple_id( 4, 5, 6 );
  auto c = make_simple_id( 0, 1, 0 );
  auto d = make_simple_id( 0, 0, 3 );
  auto e = make_simple_id( 0, 1, 3 );

  std::cout << a << std::endl;
  std::cout << b << std::endl;
  EXPECT_FALSE( decltype(a)::lexical_compare_t{}(a,a) );
  EXPECT_TRUE( decltype(a)::lexical_compare_t{}(a,b) );
  EXPECT_FALSE( decltype(a)::lexical_compare_t{}(b,a) );
  EXPECT_FALSE( (a==b) );
  EXPECT_FALSE( (b==a) );
  EXPECT_TRUE( (a==a) );
  EXPECT_FALSE( decltype(a)::lexical_compare_t{}(c,d) );
  EXPECT_TRUE( decltype(a)::lexical_compare_t{}(c,e) );
  EXPECT_TRUE( decltype(a)::lexical_compare_t{}(d,c) );
  EXPECT_TRUE( decltype(a)::lexical_compare_t{}(d,e) );

} // TEST

