/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad, LLC
   All rights reserved.
                                                                              */
#define __FLECSI_PRIVATE__
#include <flecsi/utils/ftest.h>
#include <flecsi/topology/ntree/geometry.h>

using namespace flecsi;

double rnd(const double& min,const double& max){
  static bool init = true;
  if(init){
    srand(time(NULL));
    init = false;
  }
  return static_cast<double>(rand())/static_cast<double>(RAND_MAX)*(max-min)+min;
}

using geo1d = flecsi::topology::ntree_geometry_u<double,1>;
using p1d = point_u<double,1>;

int geometry_1d_sanity(int argc, char ** argv) {

  FTEST();
  using namespace flecsi;

  // --------------------- Point inside/outside sphere -----------------------//
  p1d center(0);
  double radius = 1.5;
  // inside
  for(int i = 0 ; i < 100; ++i){
    p1d origin(rnd(center[0]-radius,center[0]+radius));
    ASSERT_TRUE(geo1d::within(origin,center,radius));
  }
  //outside
  for(int i = 0 ; i < 100; ++i){
    p1d origin(rnd(center[0]-radius*10,center[0]-radius));
    ASSERT_TRUE(!geo1d::within(origin,center,radius));
  }
  for(int i = 0 ; i < 100; ++i){
    p1d origin(rnd(center[0]+radius,center[0]+radius*10));
    ASSERT_TRUE(!geo1d::within(origin,center,radius));
  }

  // ------------------------- Sphere interaction ----------------------------//




#if 0

    //! Return true if dist^2 < radius^2
    static bool within_square(const point_t & origin,
      const point_t & center,
      element_t r1,
      element_t r2) {
      element_t x2 = (origin[0] - center[0]) * (origin[0] - center[0]);
      element_t dist_2 = x2;
      return dist_2 <= (r1 + r2) * (r1 + r2) * 0.25;
    }

    //! Return true if point origin lies within the box specified by
    //! min/max point.
    static bool within_box(const point_t & min,
      const point_t & max,
      const point_t & origin,
      const element_t & r) {
      return origin[0] <= max[0] && origin[0] >= min[0];
    }

    //! Intersection between two boxes defined by there min and max bound
    static bool intersects_box_box(const point_t & min_b1,
      const point_t & max_b1,
      const point_t & min_b2,
      const point_t & max_b2) {
      return !((max_b1[0] < min_b2[0]) || (max_b2[0] < min_b1[0]));
    }

    //! Intersection of two spheres based on center and radius
    static bool intersects_sphere_sphere(const point_t & c1,
      const element_t r1,
      const point_t & c2,
      const element_t r2) {
      return distance(c1, c2) - (r1 + r2) <= tol;
    }

    //! Intersection of sphere and box; Compute the closest point from the
    //! rectangle to the sphere and this distance less than sphere radius
    static bool intersects_sphere_box(const point_t & min,
      const point_t & max,
      const point_t & c,
      const element_t r) {
      point_t x = point_t(std::max(min[0], std::min(c[0], max[0])));
      element_t dist = distance(x, c);
      return dist - r <= tol;
    }

    /**
     * Multipole method acceptance based on MAC.
     * The angle === l/r < MAC (l source box width, r distance sink -> source)
     * Barnes & Hut 1986
     */
    bool box_MAC(const point_t & position_source,
      const point_t & position_sink,
      const point_t & box_source_min,
      const point_t & box_source_max,
      double macangle) {
      double dmax = flecsi::distance(box_source_min, box_source_max);
      double disttoc = flecsi::distance(position_sink, position_source);
      return dmax / disttoc - macangle <= tol;
    }

    #endif

}
ftest_register_driver(geometry_1d_sanity);
