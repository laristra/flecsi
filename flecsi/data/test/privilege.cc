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
#include "flecsi/util/ftest.hh"
#include <flecsi/data/privilege.hh>

using namespace flecsi;

constexpr size_t prvs1 = privilege_pack<rw>;
constexpr size_t prvs2 = privilege_pack<wo, rw>;
constexpr size_t prvs3 = privilege_pack<ro, wo, rw>;
constexpr size_t prvs4 = privilege_pack<nu, ro, wo, rw>;

int
privilege(int, char **) {
  FTEST {
    {
      static_assert(util::msb<prvs1>() == 2u);

      constexpr size_t p0 = get_privilege<0, prvs1>();

      ASSERT_EQ(p0, rw);
    } // scope

    {
      static_assert(util::msb<prvs2>() == 4u);

      constexpr size_t p0 = get_privilege<0, prvs2>();
      constexpr size_t p1 = get_privilege<1, prvs2>();

      ASSERT_EQ(p0, wo);
      ASSERT_EQ(p1, rw);
    } // scope

    {
      static_assert(util::msb<prvs3>() == 6u);

      constexpr size_t p1 = get_privilege<0, prvs3>();
      constexpr size_t p2 = get_privilege<1, prvs3>();
      constexpr size_t p3 = get_privilege<2, prvs3>();

      ASSERT_EQ(p1, ro);
      ASSERT_EQ(p2, wo);
      ASSERT_EQ(p3, rw);
    } // scope

    {
      static_assert(util::msb<prvs4>() == 8u);

      constexpr size_t p0 = get_privilege<0, prvs4>();
      constexpr size_t p1 = get_privilege<1, prvs4>();
      constexpr size_t p2 = get_privilege<2, prvs4>();
      constexpr size_t p3 = get_privilege<3, prvs4>();

      ASSERT_EQ(p0, nu);
      ASSERT_EQ(p1, ro);
      ASSERT_EQ(p2, wo);
      ASSERT_EQ(p3, rw);
    } // scope
  };
}

ftest_register_driver(privilege);
