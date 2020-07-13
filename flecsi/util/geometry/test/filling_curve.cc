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
#include "flecsi/util/geometry/filling_curve.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;

using point_t = util::point<double, 3>;
using range_t = std::array<point_t, 2>;
using hc = hilbert_curve<3, uint64_t>;
using mc = morton_curve<3, uint64_t>;

using point_2d = util::point<double, 2>;
using range_2d = std::array<point_2d, 2>;
using hc_2d = hilbert_curve<2, uint64_t>;
using mc_2d = morton_curve<2, uint64_t>;

int
hilbert_sanity() {
  UNIT {
    using namespace flecsi;

    range_t range;
    range[0] = {0, 0, 0};
    range[1] = {1, 1, 1};
    point_t p1 = {0.25, 0.25, 0.25};

    flog(info) << "Hilbert TEST " << hc::max_depth() << std::endl;

    hc hc1;
    hc hc2(range, p1);
    hc hc3 = hc::min();
    hc hc4 = hc::max();
    hc hc5 = hc::root();
    flog(info) << "Default: " << hc1 << std::endl;
    flog(info) << "Pt:rg  : " << hc2 << std::endl;
    flog(info) << "Min    : " << hc3 << std::endl;
    flog(info) << "Max    : " << hc4 << std::endl;
    flog(info) << "root   : " << hc5 << std::endl;
    ASSERT_TRUE(1 == hc5);

    while(hc4 != hc5) {
      hc4.pop();
    }
    ASSERT_TRUE(hc5 == hc4);
  };
} // hilbert_sanity

flecsi::unit::driver<hilbert_sanity> hilbert_driver;

int
hilbert_2d_rnd() {
  UNIT {
    using namespace flecsi;
    // Test the generation 2D
    range_2d rge;
    rge[0] = {0, 0};
    rge[1] = {1, 1};
    std::array<point_2d, 4> pts = {point_2d{.25, .25},
      point_2d{.25, .5},
      point_2d{.5, .5},
      point_2d{.5, .25}};
    std::array<hc_2d, 4> hcs_2d;

    for(int i = 0; i < 4; ++i) {
      hcs_2d[i] = hc_2d(rge, pts[i]);
      point_2d inv;
      hcs_2d[i].coordinates(rge, inv);
      double dist = distance(pts[i], inv);
      flog(info) << pts[i] << " " << hcs_2d[i] << " = " << inv << std::endl;
      ASSERT_TRUE(dist < 1.0e-4);
    }
  };
} // hilbert_2d_rnd

flecsi::unit::driver<hilbert_2d_rnd> hilbert_2d_rnd_driver;

int
hilbert_3d_rnd() {
  UNIT {
    using namespace flecsi;
    // Test the generation
    range_t range;

    range[0] = {0, 0, 0};
    range[1] = {1, 1, 1};
    std::array<point_t, 8> points = {point_t{.25, .25, .25},
      point_t{.25, .25, .5},
      point_t{.25, .5, .5},
      point_t{.25, .5, .25},
      point_t{.5, .5, .25},
      point_t{.5, .5, .5},
      point_t{.5, .25, .5},
      point_t{.5, .25, .25}};
    std::array<hc, 8> hcs;

    for(int i = 0; i < 8; ++i) {
      hcs[i] = hc(range, points[i]);
      point_t inv;
      hcs[i].coordinates(range, inv);
      double dist = distance(points[i], inv);
      flog(info) << points[i] << " " << hcs[i] << " = " << inv << std::endl;
      // ASSERT_TRUE(dist < 1.0e-3);
    }

    // rnd
    for(int i = 0; i < 20; ++i) {
      point_t pt((double)rand() / (double)RAND_MAX,
        (double)rand() / (double)RAND_MAX,
        (double)rand() / (double)RAND_MAX);
      point_t inv;
      hc h(range, pt);
      h.coordinates(range, inv);
      double dist = distance(pt, inv);
      flog(info) << pt << " = " << h << " = " << inv << std::endl;
      // ASSERT_TRUE(dist < 1.0e-4);
    }
  };
} // hilbert_3d_rnd

flecsi::unit::driver<hilbert_3d_rnd> hilbert_3d_rnd_driver;

int
morton_sanity() {
  UNIT {
    range_t range;
    range[0] = {-1, -1, -1};
    range[1] = {1, 1, 1};
    point_t p1 = {0, 0, 0};

    flog(info) << " Morton TEST " << hc::max_depth() << std::endl;

    mc hc1;
    mc hc2(range, p1);
    mc hc3 = mc::min();
    mc hc4 = mc::max();
    mc hc5 = mc::root();
    flog(info) << "Default: " << hc1 << std::endl;
    flog(info) << "Pt:rg  : " << hc2 << std::endl;
    flog(info) << "Min    : " << hc3 << std::endl;
    flog(info) << "Max    : " << hc4 << std::endl;
    flog(info) << "root   : " << hc5 << std::endl;
    ASSERT_TRUE(1 == hc5);

    while(hc4 != hc5) {
      hc4.pop();
    }
    ASSERT_TRUE(hc5 == hc4);
  };
}

flecsi::unit::driver<morton_sanity> morton_driver;

int
morton_2d_rnd() {
  UNIT {
    using namespace flecsi;
    // Test the generation 2d
    range_2d rge;
    rge[0] = {0, 0};
    rge[1] = {1, 1};
    std::array<point_2d, 4> pts = {point_2d{.25, .25},
      point_2d{.5, .25},
      point_2d{.25, .5},
      point_2d{.5, .5}};
    std::array<mc_2d, 4> mcs_2d;

    for(int i = 0; i < 4; ++i) {
      mcs_2d[i] = mc_2d(rge, pts[i]);
      point_2d inv;
      mcs_2d[i].coordinates(rge, inv);
      double dist = distance(pts[i], inv);
      flog(info) << pts[i] << " " << mcs_2d[i] << " = " << inv << std::endl;
      ASSERT_TRUE(dist < 1.0e-4);
    }

    // rnd
    for(int i = 0; i < 20; ++i) {
      point_2d pt(
        (double)rand() / (double)RAND_MAX, (double)rand() / (double)RAND_MAX);
      point_2d inv;
      mc_2d h(rge, pt);
      h.coordinates(rge, inv);
      double dist = distance(pt, inv);
      flog(info) << pt << " = " << h << " = " << inv << std::endl;
      ASSERT_TRUE(dist < 1.0e-4);
    }
  };
} // morton_2d_rnd

flecsi::unit::driver<morton_2d_rnd> morton_2d_rnd_driver;

int
morton_3d_rnd() {
  UNIT {
    using namespace flecsi;
    range_t range;

    // Test the generation
    range[0] = {0, 0, 0};
    range[1] = {1, 1, 1};
    std::array<point_t, 8> points = {point_t{.25, .25, .25},
      point_t{.5, .25, .25},
      point_t{.25, .5, .25},
      point_t{.5, .5, .25},
      point_t{.25, .25, .5},
      point_t{.5, .25, .5},
      point_t{.25, .5, .5},
      point_t{.5, .5, .5}};
    std::array<mc, 8> mcs;

    for(int i = 0; i < 8; ++i) {
      mcs[i] = mc(range, points[i]);
      point_t inv;
      mcs[i].coordinates(range, inv);
      double dist = distance(points[i], inv);
      flog(info) << points[i] << " " << mcs[i] << " = " << inv << std::endl;
      ASSERT_TRUE(dist < 1.0e-4);
    }

    // rnd
    for(int i = 0; i < 20; ++i) {
      point_t pt((double)rand() / (double)RAND_MAX,
        (double)rand() / (double)RAND_MAX,
        (double)rand() / (double)RAND_MAX);
      point_t inv;
      mc h(range, pt);
      h.coordinates(range, inv);
      double dist = distance(pt, inv);
      flog(info) << pt << " = " << h << " = " << inv << std::endl;
      ASSERT_TRUE(dist < 1.0e-4);
    }
  };
} // morton_3d_rnd

flecsi::unit::driver<morton_3d_rnd> morton_3d_rnd_driver;
