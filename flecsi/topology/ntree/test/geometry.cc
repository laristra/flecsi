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
  const int num_tests = 100;

  // --------------------- Point inside/outside sphere -----------------------//
  {
    p1d center(0.);
    double radius = 1.5;
    // inside
    for(int i = 0 ; i < num_tests; ++i){
      p1d origin(rnd(center[0]-radius,center[0]+radius));
      ASSERT_TRUE(geo1d::within(origin,center,radius));
    }
    //outside
    for(int i = 0 ; i < num_tests; ++i){
      p1d origin(rnd(center[0]-radius*10,center[0]-radius));
      ASSERT_TRUE(!geo1d::within(origin,center,radius));
    }
    for(int i = 0 ; i < num_tests; ++i){
      p1d origin(rnd(center[0]+radius,center[0]+radius*10));
      ASSERT_TRUE(!geo1d::within(origin,center,radius));
    }
  }

  // ------------------------- Sphere interaction ----------------------------//
  {
    p1d c1(0.);
    double r1 = 1.0;
    for(int i = 0; i < num_tests; ++i){
      p1d c2(rnd(c1[0],c1[0]+1*4));
      double r2 = rnd(0,c1[0]+r1*5);
      bool res = std::max(r1,r2) >= distance(c1,c2);
      ASSERT_EQ(res,geo1d::within_square(c1,c2,r1,r2));
    }
  }

  // -------------------------- Particle in box  -----------------------------//
  {
    p1d bmin(-1.);
    p1d bmax(1.);
    // inside
    for(int i = 0 ; i < num_tests; ++i){
      p1d center(rnd(bmin[0],bmax[0]));
      ASSERT_TRUE(geo1d::within_box(bmin,bmax,center));
    }
    // outside
    for(int i = 0 ; i < num_tests; ++i){
      p1d center(rnd(bmin[0]-10,bmin[0]));
      ASSERT_TRUE(!geo1d::within_box(bmin,bmax,center));
    }
    for(int i = 0 ; i < num_tests; ++i){
      p1d center(rnd(bmax[0],bmax[0]+10));
      ASSERT_TRUE(!geo1d::within_box(bmin,bmax,center));
    }
  }

  // ------------------------ box box intersection  --------------------------//
  {
    p1d bmin(-1.);
    p1d bmax(1.);
    p1d bmin_test[4] = {{-2},{2},{1},{-3}};
    p1d bmax_test[4] = {{2},{3},{3},{0}};
    bool res_test[4] = {1,0,1,1};
    for(int i = 0 ; i < 4; ++i){
      ASSERT_EQ(res_test[i],geo1d::intersects_box_box(bmin,bmax,bmin_test[i],bmax_test[i]));
    }
  }

  // --------------------- sphere sphere intersection  -----------------------//
  {
    p1d center(0.);
    double radius = 1;
    for(int i = 0; i < num_tests; ++i){
      p1d c = p1d(rnd(center[0]-radius,center[0]+radius));
      double r =  rnd(radius/100.,radius);
      ASSERT_TRUE(geo1d::intersects_sphere_sphere(center,radius,c,r));
    }
    for(int i = 0; i < num_tests; ++i){
      p1d c = p1d(rnd(center[0]+radius*10,center[0]+radius*20));
      double r =  rnd(radius,radius*10-1);
      ASSERT_TRUE(!geo1d::intersects_sphere_sphere(center,radius,c,r));
    }
    for(int i = 0; i < num_tests; ++i){
      p1d c = p1d(rnd(center[0]-radius*20,center[0]-radius*10));
      double r =  rnd(radius,radius*10-1);
      ASSERT_TRUE(!geo1d::intersects_sphere_sphere(center,radius,c,r));
    }
  }

  // ----------------------- sphere box intersection  ------------------------//
  {
    p1d bmin(-1.);
    p1d bmax(1.);
    p1d center_test[5] = {{0},{2},{3},{-2},{4}};
    double radius_test[5] = {1,1,1,1,5};
    bool res_test[5] = {1,1,0,1,1};
    for(int i = 0 ; i < 5; ++i){
      ASSERT_EQ(res_test[i],geo1d::intersects_sphere_box(bmin,bmax,center_test[i],radius_test[i]));
    }
  }
}
ftest_register_driver(geometry_1d_sanity);


using geo2d = flecsi::topology::ntree_geometry_u<double,2>;
using p2d = point_u<double,2>;

int geometry_2d_sanity(int argc, char ** argv) {

  FTEST();
  using namespace flecsi;
  const int num_tests = 100;

  // --------------------- Point inside/outside sphere -----------------------//
  {
    p2d center(0.,0.);
    double radius = 1.5;
    // inside
    for(int i = 0 ; i < num_tests; ++i){
      p2d origin(rnd(center[0]-radius,center[0]+radius),
                 rnd(center[1]-radius,center[1]+radius));
      bool res = distance(center,origin) <= radius;
      ASSERT_EQ(res,geo2d::within(origin,center,radius));
    }
  }

  // ------------------------- Sphere interaction ----------------------------//
  {
    p2d c1(0.,0.);
    double r1 = 1.0;
    for(int i = 0; i < num_tests; ++i){
      p2d c2(rnd(c1[0],c1[0]+1*4),rnd(c1[1],c1[1]+1*4));
      double r2 = rnd(0,c1[0]+r1*5);
      bool res = std::max(r1,r2) >= distance(c1,c2);
      ASSERT_EQ(res,geo2d::within_square(c1,c2,r1,r2));
    }
  }

  // -------------------------- Particle in box  -----------------------------//
  {
    p2d bmin(-1.,-1.);
    p2d bmax(1.,1.);
    // inside
    for(int i = 0 ; i < num_tests; ++i){
      p2d center(rnd(bmin[0],bmax[0]),rnd(bmin[1],bmax[1]));
      ASSERT_TRUE(geo2d::within_box(bmin,bmax,center));
    }
    // outside
    for(int i = 0 ; i < num_tests; ++i){
      p2d center(rnd(bmin[0]-10,bmin[0]),rnd(bmin[1]-10,bmin[1]));
      ASSERT_TRUE(!geo2d::within_box(bmin,bmax,center));
    }
    for(int i = 0 ; i < num_tests; ++i){
      p2d center(rnd(bmax[0],bmax[0]+10),rnd(bmax[1],bmax[1]+10));
      ASSERT_TRUE(!geo2d::within_box(bmin,bmax,center));
    }
  }

  // ------------------------ box box intersection  --------------------------//
  {
    p2d bmin(-1.,-1.);
    p2d bmax(1.,1.);
    p2d bmin_test[4] = {{-2,-2},{2,2},{1,1},{-3,-3}};
    p2d bmax_test[4] = {{2,2},{3,3},{3,3},{0,0}};
    bool res_test[4] = {1,0,1,1};
    for(int i = 0 ; i < 4; ++i){
      ASSERT_EQ(res_test[i],geo2d::intersects_box_box(bmin,bmax,bmin_test[i],bmax_test[i]));
    }
  }

  // --------------------- sphere sphere intersection  -----------------------//
  {
    p2d center(0.,0.);
    double radius = 1;
    for(int i = 0; i < num_tests; ++i){
      p2d c = p2d(rnd(center[0]-radius,center[0]+radius),rnd(center[0]-radius,center[0]+radius));
      double r =  rnd(radius/100.,radius);
      bool res = distance(center,c) <= radius+r;
      ASSERT_EQ(res,geo2d::intersects_sphere_sphere(center,radius,c,r));
    }
  }

  // ----------------------- sphere box intersection  ------------------------//
  {
    p2d bmin(-1.,-1.);
    p2d bmax(1.,1.);
    p2d center_test[5] = {{0,0},{2,2},{3,3},{-2,-2},{4,4}};
    double radius_test[5] = {1,1,1,1,5};
    bool res_test[5] = {1,0,0,0,1};
    for(int i = 0 ; i < 5; ++i){
      ASSERT_EQ(res_test[i],geo2d::intersects_sphere_box(bmin,bmax,center_test[i],radius_test[i]));
    }
  }
}
ftest_register_driver(geometry_2d_sanity);

using geo3d = flecsi::topology::ntree_geometry_u<double,3>;
using p3d = point_u<double,3>;

int geometry_3d_sanity(int argc, char ** argv) {

  FTEST();
  using namespace flecsi;
  const int num_tests = 100;

  // --------------------- Point inside/outside sphere -----------------------//
  {
    p3d center(0.,0.,0.);
    double radius = 1.5;
    // inside
    for(int i = 0 ; i < num_tests; ++i){
      p3d origin(rnd(center[0]-radius,center[0]+radius),
                 rnd(center[1]-radius,center[1]+radius),
                 rnd(center[2]-radius,center[2]+radius));
      bool res = distance(center,origin) <= radius;
      ASSERT_EQ(res,geo3d::within(origin,center,radius));
    }
  }

  // ------------------------- Sphere interaction ----------------------------//
  {
    p3d c1(0.,0.,0.);
    double r1 = 1.0;
    for(int i = 0; i < num_tests; ++i){
      p3d c2(rnd(c1[0],c1[0]+1*4),rnd(c1[1],c1[1]+1*4),rnd(c1[2],c1[2]+1*4));
      double r2 = rnd(0,c1[0]+r1*5);
      bool res = std::max(r1,r2) >= distance(c1,c2);
      ASSERT_EQ(res,geo3d::within_square(c1,c2,r1,r2));
    }
  }

  // -------------------------- Particle in box  -----------------------------//
  {
    p3d bmin(-1.,-1.,-1.);
    p3d bmax(1.,1.,1.);
    // inside
    for(int i = 0 ; i < num_tests; ++i){
      p3d center(rnd(bmin[0],bmax[0]),rnd(bmin[1],bmax[1]),rnd(bmin[2],bmax[2]));
      ASSERT_TRUE(geo3d::within_box(bmin,bmax,center));
    }
    // outside
    for(int i = 0 ; i < num_tests; ++i){
      p3d center(rnd(bmin[0]-10,bmin[0]),rnd(bmin[1]-10,bmin[1]),rnd(bmin[2]-10,bmin[2]));
      ASSERT_TRUE(!geo3d::within_box(bmin,bmax,center));
    }
    for(int i = 0 ; i < num_tests; ++i){
      p3d center(rnd(bmax[0],bmax[0]+10),rnd(bmax[1],bmax[1]+10),rnd(bmax[2],bmax[2]+10));
      ASSERT_TRUE(!geo3d::within_box(bmin,bmax,center));
    }
  }

  // ------------------------ box box intersection  --------------------------//
  {
    p3d bmin(-1.,-1.,-1.);
    p3d bmax(1.,1.,1.);
    p3d bmin_test[4] = {{-2,-2,-2},{2,2,2},{1,1,1},{-3,-3,-3}};
    p3d bmax_test[4] = {{2,2,2},{3,3,3},{3,3,3},{0,0,0}};
    bool res_test[4] = {1,0,1,1};
    for(int i = 0 ; i < 4; ++i){
      ASSERT_EQ(res_test[i],geo3d::intersects_box_box(bmin,bmax,bmin_test[i],bmax_test[i]));
    }
  }

  // --------------------- sphere sphere intersection  -----------------------//
  {
    p3d center(0.,0.,0.);
    double radius = 1;
    for(int i = 0; i < num_tests; ++i){
      p3d c = p3d(rnd(center[0]-radius,center[0]+radius),rnd(center[0]-radius,center[0]+radius),rnd(center[2]-radius,center[2]+radius));
      double r =  rnd(radius/100.,radius);
      bool res = distance(center,c) <= radius+r;
      ASSERT_EQ(res,geo3d::intersects_sphere_sphere(center,radius,c,r));
    }
  }

  // ----------------------- sphere box intersection  ------------------------//
  {
    p3d bmin(-1.,-1.,-1.);
    p3d bmax(1.,1.,1.);
    p3d center_test[5] = {{0,0,0},{2,2,2},{3,3,3},{-2,-2,-2},{2,2,2}};
    double radius_test[5] = {1,1,1,1,5};
    bool res_test[5] = {1,0,0,0,1};
    for(int i = 0 ; i < 5; ++i){
      ASSERT_EQ(res_test[i],geo3d::intersects_sphere_box(bmin,bmax,center_test[i],radius_test[i]));
    }
  }
}
ftest_register_driver(geometry_3d_sanity);
