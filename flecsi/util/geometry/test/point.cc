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
#include "flecsi/util/geometry/point.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;

using point_1d_t = util::point<double, 1>;
using point_2d_t = util::point<double, 2>;
using point_3d_t = util::point<double, 3>;

int
point_sanity() {
  UNIT {
    constexpr point_1d_t a1{-1.0};
    static_assert(-1.0 == a1[util::axis::x]);

    constexpr point_2d_t a2{3.0, 0.0};
    static_assert(3.0 == a2[util::axis::x]);
    static_assert(0.0 == a2[util::axis::y]);

    constexpr point_3d_t a3{3.0, 0.0, -1.0};
    static_assert(3.0 == a3[util::axis::x]);
    static_assert(0.0 == a3[util::axis::y]);
    static_assert(-1.0 == a3[util::axis::z]);
  };
} // point_sanity

flecsi::unit::driver<point_sanity> point_sanity_driver;

int
point_distance() {
  UNIT {
    point_1d_t a1{1.0};
    point_1d_t b1{4.0};
    double d = distance(a1, b1);
    ASSERT_EQ(3.0, d) << "Distance calculation failed";

    point_2d_t a2{1.0, 2.0};
    point_2d_t b2{4.0, 6.0};
    d = distance(a2, b2);
    ASSERT_EQ(5.0, d) << "Distance calculation failed";

    point_3d_t a3{1.0, 2.0, -1.0};
    point_3d_t b3{4.0, 6.0, -1.0 - std::sqrt(11.0)};
    d = distance(a3, b3);
    ASSERT_EQ(6.0, d) << "Distance calculation failed";
  };
} // point_distance

flecsi::unit::driver<point_distance> point_distance_driver;

int
point_midpoint() {
  UNIT {
    point_1d_t a1{1.0};
    point_1d_t b1{4.0};
    point_1d_t c1 = midpoint(a1, b1);
    ASSERT_EQ(2.5, c1[0]) << "Midpoint calculation failed";

    point_2d_t a2{1.0, 2.0};
    point_2d_t b2{4.0, 6.0};
    point_2d_t c2 = midpoint(a2, b2);
    ASSERT_EQ(2.5, c2[0]) << "Midpoint calculation failed";
    ASSERT_EQ(4.0, c2[1]) << "Midpoint calculation failed";

    point_3d_t a3{1.0, 2.0, -1.0};
    point_3d_t b3{4.0, 6.0, -4.0};
    point_3d_t c3 = midpoint(a3, b3);
    ASSERT_EQ(2.5, c3[0]) << "Midpoint calculation failed";
    ASSERT_EQ(4.0, c3[1]) << "Midpoint calculation failed";
    ASSERT_EQ(-2.5, c3[2]) << "Midpoint calculation failed";
  };
} // point_midpoint

flecsi::unit::driver<point_midpoint> point_midpoint_driver;
