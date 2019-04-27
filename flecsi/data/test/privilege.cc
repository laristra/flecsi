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

using privileges = privilege_pack_u<nu, ro, wo, rw>;

int
privilege(int argc, char ** argv) {

  FTEST();

  ASSERT_EQ(utils::msb<privileges::value>(), 9);

  constexpr size_t p0 = get_privilege<0, privileges::value>();
  constexpr size_t p1 = get_privilege<1, privileges::value>();
  constexpr size_t p2 = get_privilege<2, privileges::value>();
  constexpr size_t p3 = get_privilege<3, privileges::value>();

  ASSERT_EQ(p0, nu);
  ASSERT_EQ(p1, ro);
  ASSERT_EQ(p2, wo);
  ASSERT_EQ(p3, rw);

  return 0;
}

ftest_register_test(privilege);
