/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "../../specializations/burton.h"
#include "../io.h"

#include <vector>

using namespace std;
using namespace flexi;

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;

// test fixture for creating the mesh
class Burton : public ::testing::Test {
protected:
  virtual void SetUp() {
    vector<vertex_t*> vs;
  
    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
	auto v =
	  b.create_vertex({double(i)+ 0.1*pow(double(j),1.8), 1.5*double(j)});
	v->setRank(1);
	vs.push_back(v);
      }
    }

    size_t width1 = width + 1;
    for(size_t j = 0; j < height; ++j){
      for(size_t i = 0; i < width; ++i){
	// go over vertices counter clockwise to define cell
	auto c = 
	  b.create_cell({vs[i + j * width1],
            vs[i + 1 + j * width1],
            vs[i + 1 + (j + 1) * width1],
            vs[i + (j + 1) * width1]});
      }
    }

    b.init();
  }

  virtual void TearDown() { }

  burton_mesh_t b;
  const size_t width = 10;
  const size_t height = 20;
};

using real_t = burton_mesh_t::real_t;
using vector_t = burton_mesh_t::vector_t;
TEST_F(Burton, write_exo) {
  // create state data on b
  // register
  register_state(b, "pressure", cells, real_t, persistent);
  register_state(b, "velocity", vertices, vector_t, persistent);
  // access
  auto p = access_state(b, "pressure", cells, real_t);
  auto velocity = access_state(b, "velocity", vertices, vector_t);
  // initialize
  for(auto c: p) {
    p[c] = c;
  } // for
  // vertices
  for (auto v: velocity) {
    velocity[v][0] = v;
    velocity[v][1] = 2.0*v;
  } // for

  // write the mesh
  std::string filename("test/mesh.exo");
  ASSERT_FALSE(write_mesh(filename, b));

} // TEST_F

TEST_F(Burton, write_g) {
  std::string filename("test/mesh.g");
  ASSERT_FALSE(write_mesh(filename, b));
} // TEST_F

TEST_F(Burton, read_exo) {
  burton_mesh_t m;
  // read mesh written by above test
  std::string filename("test/mesh.exo");
  ASSERT_FALSE(read_mesh(filename, m));

  // write m to a different file
  filename = "test/mesh_out.exo";
  ASSERT_FALSE(write_mesh(filename, m));
} // TEST_F

TEST_F(Burton, read_g) {
  burton_mesh_t m;
  // read mesh written by above test
  std::string filename("test/mesh.g");
  ASSERT_FALSE(read_mesh(filename, m));

  // write m to a different file
  filename = "test/mesh_out.g";
  ASSERT_FALSE(write_mesh(filename, m));
} // TEST_F

/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << std::endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
