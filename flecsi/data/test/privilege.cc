/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#define __FLECSI_PRIVATE__
#include <flecsi/data/common/privilege.h>
#include <flecsi/utils/ftest.h>
#include <flecsi/utils/typeify.h>

using namespace flecsi;

using prvs1 = privilege_pack_u<rw>;
using prvs2 = privilege_pack_u<wo, rw>;
using prvs3 = privilege_pack_u<ro, wo, rw>;
using prvs4 = privilege_pack_u<nu, ro, wo, rw>;

int
privilege(int argc, char ** argv) {

  FTEST();

  {
  ASSERT_EQ(utils::msb<prvs1::value>(), 3);

  constexpr size_t p0 = get_privilege<0, prvs1::value>();

  ASSERT_EQ(p0, rw);
  } // scope

  {
  ASSERT_EQ(utils::msb<prvs2::value>(), 5);

  constexpr size_t p0 = get_privilege<0, prvs2::value>();
  constexpr size_t p1 = get_privilege<1, prvs2::value>();

  ASSERT_EQ(p0, wo);
  ASSERT_EQ(p1, rw);
  } // scope

  {
  ASSERT_EQ(utils::msb<prvs3::value>(), 7);

  constexpr size_t p1 = get_privilege<0, prvs3::value>();
  constexpr size_t p2 = get_privilege<1, prvs3::value>();
  constexpr size_t p3 = get_privilege<2, prvs3::value>();

  ASSERT_EQ(p1, ro);
  ASSERT_EQ(p2, wo);
  ASSERT_EQ(p3, rw);
  } // scope

  {
  ASSERT_EQ(utils::msb<prvs4::value>(), 9);

  constexpr size_t p0 = get_privilege<0, prvs4::value>();
  constexpr size_t p1 = get_privilege<1, prvs4::value>();
  constexpr size_t p2 = get_privilege<2, prvs4::value>();
  constexpr size_t p3 = get_privilege<3, prvs4::value>();

  ASSERT_EQ(p0, nu);
  ASSERT_EQ(p1, ro);
  ASSERT_EQ(p2, wo);
  ASSERT_EQ(p3, rw);
  } // scope

  return 0;
}

ftest_register_test(privilege);
