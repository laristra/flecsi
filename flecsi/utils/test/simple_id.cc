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
#include <cinch/ctest.h>

using namespace flecsi::utils;

using id_types_t = std::tuple<int,int,int>;
using my_id_t = simple_id_t< id_types_t, lexical_comparison<id_types_t> >; 


// TEST
TEST(simple_id, all) {

  auto a = my_id_t{ 1, 2, 3 };
  auto b = my_id_t{ 4, 5, 6 };
  auto c = my_id_t{ 0, 1, 0 };
  auto d = my_id_t{ 0, 0, 3 };
  auto e = my_id_t{ 0, 1, 3 };

  std::cout << a << std::endl;
  std::cout << b << std::endl;
  EXPECT_FALSE( (a<a) );
  EXPECT_TRUE( (a<b) );
  EXPECT_FALSE( (b<a) );
  EXPECT_FALSE( (a==b) );
  EXPECT_FALSE( (b==a) );
  EXPECT_TRUE( (a==a) );
  EXPECT_FALSE( (c<d) );
  EXPECT_TRUE( (c<e) );
  EXPECT_TRUE( (d<c) );
  EXPECT_TRUE( (d<e) );

} // TEST

