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

using point_2d = point_u<double,2>;
using range_2d = std::array<point_2d,2>;
using hc_2d = hilbert_curve_u<2,uint64_t>;
using mc_2d = morton_curve_u<2,uint64_t>;

int hilbert_sanity(int argc, char ** argv) {

  FTEST();
  using namespace flecsi;

  range_t range;
  range[0] = {0, 0, 0};
  range[1] = {1, 1, 1};
  point_t p1 = {0.25, 0.25, 0.25};

  flog(info) << "Hilbert TEST " << hc::max_depth() << std::endl;

  hc::set_range(range);
  hc hc1;
  hc hc2(p1);
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

}
ftest_register_test(hilbert_sanity);

int hilbert_2d_rnd(int argc, char ** argv){
  FTEST();
  using namespace flecsi;
  // Test the generation 2D
  range_2d rge;
  rge[0] = {0,0};
  rge[1] = {1,1};
  hc_2d::set_range(rge);
  std::array<point_2d,4> pts = {
    point_2d{.25,.25},point_2d{.25,.5},point_2d{.5,.5},point_2d{.5,.25}};
  std::array<hc_2d,4> hcs_2d;

  for(int i = 0 ; i < 4; ++i){
    hcs_2d[i] = hc_2d(pts[i]);
    point_2d inv;
    hcs_2d[i].coordinates(inv);
    double dist = distance(pts[i],inv);
    flog(info) << pts[i] <<" "<< hcs_2d[i] << " = "<<inv<<std::endl;
    ASSERT_TRUE(dist<1.0e-4);
  }
}
ftest_register_test(hilbert_2d_rnd);

int hilbert_3d_rnd(int argc, char ** argv){
  FTEST();
  using namespace flecsi;
  // Test the generation
  range_t range;

  range[0] = {0,0,0};
  range[1] = {1,1,1};
  hc::set_range(range);
  std::array<point_t,8> points = {
    point_t{.25,.25,.25},point_t{.25,.25,.5},point_t{.25,.5,.5},point_t{.25,.5,.25},
    point_t{.5,.5,.25},point_t{.5,.5,.5},point_t{.5,.25,.5},point_t{.5,.25,.25}};
  std::array<hc,8> hcs;

  for(int i = 0 ; i < 8; ++i){
    hcs[i] = hc(points[i]);
    point_t inv;
    hcs[i].coordinates(inv);
    double dist = distance(points[i],inv);
    flog(info) << points[i] <<" "<< hcs[i] << " = "<<inv<<std::endl;
    ASSERT_TRUE(dist<1.0e-4);
  }

  // rnd
  for(int i = 0 ; i < 20; ++i){
    point_t pt(
      (double)rand()/(double)RAND_MAX,
      (double)rand()/(double)RAND_MAX,
      (double)rand()/(double)RAND_MAX);
    point_t inv;
    hc h(pt);
    h.coordinates(inv);
    double dist = distance(pt,inv);
    flog(info) << pt <<" = "<< h << " = "<<inv<<std::endl;
    ASSERT_TRUE(dist<1.0e-4);
  }
} // TEST
ftest_register_test(hilbert_3d_rnd);

int morton_sanity(int argc, char ** argv) {

  FTEST();

  range_t range;
  range[0] = {-1, -1, -1};
  range[1] = {1, 1, 1};
  point_t p1 = {0, 0, 0};

  flog(info) <<" Morton TEST "<< hc::max_depth() << std::endl;

  mc::set_range(range);
  mc hc1;
  mc hc2(p1);
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

}

int morton_2d_rnd(int argc, char ** argv){
  FTEST();
  using namespace flecsi;
  // Test the generation 2d
  range_2d rge;
  rge[0] = {0,0};
  rge[1] = {1,1};
  mc_2d::set_range(rge);
  std::array<point_2d,4> pts = {
    point_2d{.25,.25},point_2d{.5,.25},point_2d{.25,.5},point_2d{.5,.5}};
  std::array<mc_2d,4> mcs_2d;

  for(int i = 0 ; i < 4; ++i){
    mcs_2d[i] = mc_2d(pts[i]);
    point_2d inv;
    mcs_2d[i].coordinates(inv);
    double dist = distance(pts[i],inv);
    flog(info) << pts[i] <<" "<< mcs_2d[i] << " = "<<inv<<std::endl;
    ASSERT_TRUE(dist<1.0e-4);
  }

  // rnd
  for(int i = 0 ; i < 20; ++i){
    point_2d pt(
      (double)rand()/(double)RAND_MAX,
      (double)rand()/(double)RAND_MAX);
    point_2d inv;
    mc_2d h(pt);
    h.coordinates(inv);
    double dist = distance(pt,inv);
    flog(info) << pt <<" = "<< h << " = "<<inv<<std::endl;
    ASSERT_TRUE(dist<1.0e-4);
  }
}

ftest_register_test(morton_2d_rnd);

int morton_3d_rnd(int argc, char ** argv){
  FTEST();
  using namespace flecsi;
  range_t range;

  // Test the generation
  range[0] = {0,0,0};
  range[1] = {1,1,1};
  mc::set_range(range);
  std::array<point_t,8> points = {
    point_t{.25,.25,.25},point_t{.5,.25,.25},point_t{.25,.5,.25},point_t{.5,.5,.25},
    point_t{.25,.25,.5},point_t{.5,.25,.5},point_t{.25,.5,.5},point_t{.5,.5,.5}};
  std::array<mc,8> mcs;

  for(int i = 0 ; i < 8; ++i){
    mcs[i] = mc(points[i]);
    point_t inv;
    mcs[i].coordinates(inv);
    double dist = distance(points[i],inv);
    flog(info) << points[i] <<" "<< mcs[i] << " = "<<inv<<std::endl;
    ASSERT_TRUE(dist<1.0e-4);
  }

  // rnd
  for(int i = 0 ; i < 20; ++i){
    point_t pt(
      (double)rand()/(double)RAND_MAX,
      (double)rand()/(double)RAND_MAX,
      (double)rand()/(double)RAND_MAX);
    point_t inv;
    mc h(pt);
    h.coordinates(inv);
    double dist = distance(pt,inv);
    flog(info) << pt <<" = "<< h << " = "<<inv<<std::endl;
    ASSERT_TRUE(dist<1.0e-4);
  }
}
ftest_register_test(morton_3d_rnd);

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
