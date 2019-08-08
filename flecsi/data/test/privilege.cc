/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#define __FLECSI_PRIVATE__
#include <flecsi/data/common/privilege.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

constexpr size_t prvs1 = privilege_pack<rw>::value;
constexpr size_t prvs2 = privilege_pack<wo, rw>::value;
constexpr size_t prvs3 = privilege_pack<ro, wo, rw>::value;
constexpr size_t prvs4 = privilege_pack<nu, ro, wo, rw>::value;

int
privilege(int argc, char ** argv) {

  FTEST();

  {
    ASSERT_EQ(utils::msb<prvs1>(), 3);

    constexpr size_t p0 = get_privilege<0, prvs1>();

    ASSERT_EQ(p0, rw);
  } // scope

  {
    ASSERT_EQ(utils::msb<prvs2>(), 5);

    constexpr size_t p0 = get_privilege<0, prvs2>();
    constexpr size_t p1 = get_privilege<1, prvs2>();

    ASSERT_EQ(p0, wo);
    ASSERT_EQ(p1, rw);
  } // scope

  {
    ASSERT_EQ(utils::msb<prvs3>(), 7);

    constexpr size_t p1 = get_privilege<0, prvs3>();
    constexpr size_t p2 = get_privilege<1, prvs3>();
    constexpr size_t p3 = get_privilege<2, prvs3>();

    ASSERT_EQ(p1, ro);
    ASSERT_EQ(p2, wo);
    ASSERT_EQ(p3, rw);
  } // scope

  {
    ASSERT_EQ(utils::msb<prvs4>(), 9);

    constexpr size_t p0 = get_privilege<0, prvs4>();
    constexpr size_t p1 = get_privilege<1, prvs4>();
    constexpr size_t p2 = get_privilege<2, prvs4>();
    constexpr size_t p3 = get_privilege<3, prvs4>();

    ASSERT_EQ(p0, nu);
    ASSERT_EQ(p1, ro);
    ASSERT_EQ(p2, wo);
    ASSERT_EQ(p3, rw);
  } // scope

  return FTEST_RESULT();
}

ftest_register_driver(privilege);
