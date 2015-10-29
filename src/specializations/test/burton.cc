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

#include "../burton.h"

#include <vector>

using namespace std;
using namespace flexi;

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;
using vector_t = burton_mesh_t::vector_t;

// test fixture for creating the mesh
class Burton : public ::testing::Test {
protected:
  virtual void SetUp() {
    vector<vertex_t*> vs;
  
    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
	auto v = b.create_vertex({double(i), double(j)});
	v->set_rank(1);
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
  const size_t width = 2;
  const size_t height = 2;
};

TEST_F(Burton, mesh) {
  for(auto v : b.vertex_ents()){
    CINCH_CAPTURE() << "----------- vertex: " << v->id() << endl;
  }

  for(auto e : b.edge_ents()){
    CINCH_CAPTURE() << "----------- edge: " << e->id() << endl;
  }

  for(auto c : b.cell_ents()){
    CINCH_CAPTURE() << "----------- cell: " << c->id() << endl;
    for(auto e : b.edge_ents(c)){
      CINCH_CAPTURE() << "++++ edge of: " << e->id() << endl;
    }
    for(auto w : c->wedges()){
      CINCH_CAPTURE() << "++++ wedge of: " << w->id() << endl;
      CINCH_CAPTURE() << "### corner of: " << w->corner()->id() << endl;
    }

    for(auto c2 : c->corners()){
      CINCH_CAPTURE() << "++++ corner of: " << c2->id() << endl;
      for(auto w : c2->wedges()){
        CINCH_CAPTURE() << "++ wedge of: " << w->id() << endl;
        for(auto v : b.vertex_ents(w)){
          CINCH_CAPTURE() << "- vertex of: " << v->id() << endl;
        }
      }
    }
  }

  CINCH_ASSERT(TRUE, CINCH_EQUAL_BLESSED("burton.blessed"));

} // TEST_F

TEST_F(Burton, coordinates) {

  for(auto c: b.cell_ents()) {
    //auto xc = c->coordinates();
    cout << "---- cell " << c->id() << endl;
    for(auto v : b.vertex_ents(c)){
      auto xv = v->coordinates();
      cout << "++++ vertex " << v->id();
      cout << ": (" << xv[0] << "," << xv[1] << ")" << endl;
    } // for
  } // for
} // TEST_F

using real_t = burton_mesh_t::real_t;

TEST_F(Burton, state) {

  register_state(b, "pressure", cells, real_t, persistent);
  register_state(b, "velocity", vertices, vector_t, persistent);
  register_state(b, "H", edges, vector_t);
  register_state(b, "cornerdata", corners, int32_t);
  register_state(b, "wedgedata", wedges, bool);

  auto p = access_state(b, "pressure", real_t);
  auto velocity = access_state(b, "velocity", vector_t);
  auto H = access_state(b, "H", vector_t);
  auto cd = access_state(b, "cornerdata", int32_t);
  auto wd = access_state(b, "wedgedata", bool);

  // cells
  for(auto c: p) {
    p[c] = c;
  } // for

  for(auto c: p) {
    ASSERT_EQ(c, p[c]);
  } // for

  // vertices
  for (auto v: velocity) {
    velocity[v][0] = v;
    velocity[v][1] = 2.0*v;
  } // for

  for (auto v: velocity) {
    ASSERT_EQ(v, velocity[v][0]);
    ASSERT_EQ(2.0*v, velocity[v][1]);
  } // for

  // edges
  for (auto e: H) {
    H[e][0] = e*e;
    H[e][1] = e*e*e;
  } // for

  for (auto e: H) {
    ASSERT_EQ(e*e, H[e][0]);
    ASSERT_EQ(e*e*e, H[e][1]);
  } // for

  std::cerr << "num_corners " << b.num_corners() << std::endl;

  // corners
  for (auto c: cd) {
    cd[c] = c;
  } // for

  for (auto c: cd) {
    ASSERT_EQ(c, cd[c]);
  } // for

  std::cerr << "num_wedges " << b.num_wedges() << std::endl;

  // wedges
  for (auto w: wd) {
    wd[w] = (w%2) ? true : false;
  } // for

  for (auto w: wd) {
    if (w%2) {
      ASSERT_TRUE(wd[w]);
    }
    else {
      ASSERT_FALSE(wd[w]);
    }
  } // for

} // TEST_F

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
