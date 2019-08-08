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
#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/geometry/point.hh>

using namespace flecsi;

using point_1d_t = point<double, 1>;
using point_2d_t = point<double, 2>;
using point_3d_t = point<double, 3>;

int
ntree_sanity(int argc, char ** argv) {
  FTEST();
  return 0;
} // TEST
