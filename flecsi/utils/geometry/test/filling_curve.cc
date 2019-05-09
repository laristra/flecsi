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
#include <flecsi/utils/ftest.h>
#include <flecsi/utils/geometry/filling_curve.h>

using namespace flecsi;

using point_t = point_u<double, 3>;
using range_t = std::array<point_t, 2>;
using hc = hilbert_curve_u<3, uint64_t>;
using mc = morton_curve_u<3, uint64_t>;


int
hilbert(int argc, char ** argv) {

  FTEST();
  using namespace flecsi;

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


int
morton(int argc, char ** argv) {

  FTEST();

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

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
