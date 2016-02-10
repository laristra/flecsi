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

    size_t max_rank = (height+1)*(width+1) - 1;

    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
        auto v = b.create_vertex({double(i), double(j)});
        vs.push_back(v);
      } // for
    } // for

    size_t width1 = width + 1;
    for(size_t j = 0; j < height; ++j){
      for(size_t i = 0; i < width; ++i){
        // go over vertices counter clockwise to define cell
        auto c = b.create_cell({vs[i + j * width1],
            vs[i + 1 + j * width1],
            vs[i + 1 + (j + 1) * width1],
            vs[i + (j + 1) * width1]});
      } // for
    } // for

    b.init();
  } // SetUp

  virtual void TearDown() { }

  burton_mesh_t b;
  const size_t width = 2;
  const size_t height = 2;
};

TEST_F(Burton, dump) {
  b.dump();
}

TEST_F(Burton, mesh) {
  std::string separator;
  separator.insert(0,80,'=');
  separator.append("\n");

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "Vertices in mesh:" << std::endl;
  for(auto v : b.vertices()){
    CINCH_CAPTURE() << "----------- vertex id: " << v.id()
      << " with coordinates " << v->coordinates() << endl;
  }

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "Edges in mesh:" << std::endl;
  for(auto e : b.edges()){
    CINCH_CAPTURE() << "----------- edge id: " << e.id()
      << " with midpoint " << e->midpoint() << endl;
  }

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "Corners in mesh:" << std::endl;

  for(auto c : b.corners()) {
    CINCH_CAPTURE() << "----------- corner id: " << c.id() << endl;

    for(auto e: b.edges(c)) {
      CINCH_CAPTURE() << "       ++++ edge id: " << e.id() << endl;
    } // for
  } // for

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "For each cell:" << std::endl;
  for(auto c : b.cells()) {
    CINCH_CAPTURE() << "-----------Edges for cell id: " << c.id()
      << " with centroid " << c->centroid() << endl;
    for(auto e : b.edges(c)){
      CINCH_CAPTURE() << "++++ edge id: " << e.id()
        << " with midpoint " << e->midpoint() << endl;
    }
#if 0
    for(auto w : c->wedges()){
      CINCH_CAPTURE() << "++++ wedge of: " << w.id() << endl;
      CINCH_CAPTURE() << "### corner of: " << w->corner().id() << endl;
    }

    for(auto c2 : c->corners()){
      CINCH_CAPTURE() << "++++ corner of: " << c2.id() << endl;
      for(auto w : c2->wedges()){
        CINCH_CAPTURE() << "++ wedge of: " << w.id() << endl;
        for(auto v : b.vertices(w)){
          CINCH_CAPTURE() << "- vertex of: " << v.id() << endl;
        }
      }
    }
#endif
  }

  //CINCH_ASSERT(TRUE, CINCH_EQUAL_BLESSED("burton.blessed"));

  std::cout << CINCH_DUMP() << std::endl;

} // TEST_F

#if 0
TEST_F(Burton, coordinates) {

  for(auto c: b.cells()) {
    auto xc = b.centroid(c);
    cout << "---- cell " << c.id() << endl;
    for(auto v : b.vertices(c)){
      auto xv = v->coordinates();
      cout << "++++ vertex " << v.id();
      cout << ": (" << xv[0] << "," << xv[1] << ")" << endl;
    } // for
  } // for
} // TEST_F
#endif

TEST_F(Burton, accessors) {
  register_state(b, "pressure", cells, real_t, persistent);
  register_state(b, "density", cells, real_t);
  register_state(b, "total energy", cells, real_t, persistent);
  register_state(b, "velocity", vertices, vector_t, persistent);
  register_state(b, "H", edges, vector_t);

  std::cout << "Accessing state with type real_t:" << std::endl;

  auto vr = access_type(b, real_t);
  for(auto v: vr) {
    std::cout << "\t" << v.label() << " has type real_t" << std::endl;
  } // for

  std::cout << std::endl;

  std::cout << "Accessing state with type real_t at cells:" << std::endl;

  auto va = access_type_if(b, real_t, is_at(cells));
  for(auto v: va) {
    std::cout << "\t" << v.label() <<
      " has type real_t and is at cells" << std::endl;
  } // for

  std::cout << std::endl;

  std::cout << "Accessing persistent state with type real_t at cells:" <<
    std::endl;

  auto vp = access_type_if(b, real_t, is_persistent_at(cells));

  for(auto v: vp) {
    std::cout << "\t" << v.label() <<
      " has type real_t and is persistent at cells" << std::endl;
  } // for

  std::cout << std::endl;

  std::cout << "Accessing state with type vector_t at vertices:" << std::endl;

  auto vv = access_type_if(b, vector_t, is_at(vertices));
  for(auto v: vv) {
    std::cout << "\t" << v.label() <<
      " has type vector_t and is at vertices" << std::endl;
  } // for

  std::cout << std::endl;

  std::cout << "Accessing persistent state with type vector_t at vertices:"
            << std::endl;

  auto vpv = access_type_if(b, vector_t, is_persistent_at(vertices));
  for(auto v: vpv) {
    std::cout << "\t" << v.label() <<
      " has type vector_t and is persistent at vertices" << std::endl;
  } // for

  std::cout << std::endl;

  std::cout << "Accessing state with type vector_t at edges:" << std::endl;

  auto ve = access_type_if(b, vector_t, is_at(edges));
  for(auto v: ve) {
    std::cout << "\t" << v.label() <<
      " has type vector_t and is at edges" << std::endl;
  } // for

  std::cout << std::endl;

  std::cout << "Accessing state with type point_t at vertices:" << std::endl;

  auto pv = access_type_if(b, point_t, is_at(vertices));
  for(auto v: pv) {
    std::cout << "\t" << v.label() <<
      " has type point_t and is at vertices" << std::endl;
  } // for

  std::cout << std::endl;

  std::cout << "Accessing persistent state with type point_t at vertices:"
            << std::endl;

  auto ppv = access_type_if(b, point_t, is_persistent_at(vertices));
  for(auto v: ppv) {
    std::cout << "\t" << v.label() <<
      " has type point_t and is persistent at vertices" << std::endl;
  } // for

  std::cout << std::endl;

} // TEST_F

TEST_F(Burton, state) {

  register_state(b, "pressure", cells, real_t, persistent);
  register_state(b, "velocity", vertices, vector_t, persistent);
  register_state(b, "H", edges, vector_t);
//  register_state(b, "cornerdata", corners, int32_t);
//  register_state(b, "wedgedata", wedges, bool);

  auto p = access_state(b, "pressure", real_t);
  auto velocity = access_state(b, "velocity", vector_t);
  auto H = access_state(b, "H", vector_t);
//  auto cd = access_state(b, "cornerdata", int32_t);
//  auto wd = access_state(b, "wedgedata", bool);

  // cells
  ASSERT_EQ(4, b.num_cells());
  for(auto c: b.cells()) {
    p[c] = c.id();
  } // for

  for(auto c: b.cells()) {
    ASSERT_EQ(c.id(), p[c]);
  } // for

  // vertices
  ASSERT_EQ(9, b.num_vertices());
  for (auto v: b.vertices()) {
    velocity[v][0] = v.id();
    velocity[v][1] = 2.0*v.id();
  } // for

  for (auto v: b.vertices()) {
    ASSERT_EQ(v.id(), velocity[v][0]);
    ASSERT_EQ(2.0*v.id(), velocity[v][1]);
  } // for

  // edges
  ASSERT_EQ(12, b.num_edges());
  for (auto e: b.edges()) {
    H[e][0] = e.id()*e.id();
    H[e][1] = e.id()*e.id()*e.id();
  } // for

  for (auto e: b.edges()) {
    ASSERT_EQ(e.id()*e.id(), H[e][0]);
    ASSERT_EQ(e.id()*e.id()*e.id(), H[e][1]);
  } // for

#if 0
  // corners
  std::cerr << "num_corners " << b.num_corners() << std::endl;
  ASSERT_EQ(5, b.num_corners());
//FIXME: Need to implement mesh entities
  for (auto c: b.corners()) {
    cd[c] = c.id();
  } // for

  for (auto c: b.corners()) {
    ASSERT_EQ(c.id(), cd[c]);
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
#endif
} // TEST_F

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
