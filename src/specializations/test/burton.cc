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

// test fixture for creating the mesh
class Burton : public ::testing::Test {
protected:
  virtual void SetUp() {
    vector<vertex_t*> vs;
  
    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
	auto v = b.create_vertex({double(i), double(j)});
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
  const size_t width = 2;
  const size_t height = 2;
};

TEST_F(Burton, mesh) {
  for(auto v : b.vertices()){
    CINCH_CAPTURE() << "----------- vertex: " << v->id() << endl;
  }

  for(auto e : b.edges()){
    CINCH_CAPTURE() << "----------- edge: " << e->id() << endl;
  }

  for(auto c : b.cells()){
    CINCH_CAPTURE() << "----------- cell: " << c->id() << endl;
    for(auto e : b.edges(c)){
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
        for(auto v : b.vertices(w)){
          CINCH_CAPTURE() << "- vertex of: " << v->id() << endl;
        }
      }
    }
  }

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("burton.blessed"));

} // TEST_F

TEST_F(Burton, coordinates) {

  for(auto c: b.cells()) {
    //auto xc = c->coordinates();
    cout << "---- cell " << c->id() << endl;
    for(auto v : b.vertices(c)){
      auto xv = v->coordinates();
      cout << "++++ vertex " << v->id();
      cout << ": (" << xv[0] << "," << xv[1] << ")" << endl;
    } // for
  } // for
} // TEST_F

TEST_F(Burton, state) {

  register_state(b, "pressure", vertices, double);

  auto p = access_state(b, "pressure", vertices, double);

  for(auto v: p) {
    p[v] = v;
  } // for

  for(auto v: p) {
    std::cout << "pressure " << v << ": " << p[v] << std::endl;
  } // for
} // TEST_F

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
