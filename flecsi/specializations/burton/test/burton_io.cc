/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/specializations/burton/burton.h"
#include "flecsi/io/io.h"
#include "flecsi/specializations/burton/burton_io_exodus.h"

#include <vector>

using namespace std;
using namespace flecsi;

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;
using vector_t = burton_mesh_t::vector_t;
using real_t = burton_mesh_t::real_t;

// test fixture for creating the mesh
class Burton : public ::testing::Test {
protected:
  virtual void SetUp() {
    vector<vertex_t*> vs;

    b.init_parameters((height+1)*(width+1));

    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
	auto v =
	  b.create_vertex({double(i)+0.1*pow(double(j),1.8), 1.5*double(j)});
//	v->set_rank(1);
	vs.push_back(v);
      }
    }

    size_t width1 = width + 1;
    for(size_t j = 0; j < height; ++j){
      for(size_t i = 0; i < width; ++i){
	      // go over vertices counter clockwise to define cell
        b.create_cell({vs[i + j * width1],
          vs[i + 1 + j * width1],
          vs[i + 1 + (j + 1) * width1],
          vs[i + (j + 1) * width1]});
      } // for
    } // for

    b.init();
  }

  virtual void TearDown() { }

  burton_mesh_t b;
  const size_t width = 4;
  const size_t height = 8;
};

TEST_F(Burton, write_exo) {
  // create state data on b
  // register
  register_state(b, "pressure", cells, real_t, persistent);
  register_state(b, "region", cells, int, persistent);
  register_state(b, "velocity", vertices, vector_t, persistent);
  // access
  auto p = access_state(b, "pressure", real_t);
  auto r = access_state(b, "region", int);
  auto velocity = access_state(b, "velocity", vector_t);
  // initialize
  for(auto c: b.cells()) {
    p[c] = c.id();
    r[c] = b.num_cells() - c.id();
  } // for
  // vertices
  for (auto v: b.vertices()) {
    velocity[v][0] = v.id();
    velocity[v][1] = 2.0*v.id();
  } // for

  // write the mesh
  std::string name("mesh.exo");
  ASSERT_FALSE(write_mesh(name, b));

} // TEST_F

TEST_F(Burton, read_exo) {
  burton_mesh_t m;
  // read mesh written by above test
  std::string name("mesh.exo");
  ASSERT_FALSE(read_mesh(name, m));

  // write m to a different file
  name = "mesh_out.exo";
  ASSERT_FALSE(write_mesh(name, m));
} // TEST_F

TEST_F(Burton, write_g) {
  std::string name("mesh.g");
  ASSERT_FALSE(write_mesh(name, b));
} // TEST_F

TEST_F(Burton, read_g) {
  burton_mesh_t m;
  // read mesh written by above test
  std::string name("mesh.g");
  ASSERT_FALSE(read_mesh(name, m));

  // write m to a different file
  name = "mesh_out.g";
  ASSERT_FALSE(write_mesh(name, m));
} // TEST_F

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
