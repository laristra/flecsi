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
  std::string separator;
  separator.insert(0,80,'=');
  separator.append("\n");

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "dump" << endl;
  CINCH_CAPTURE() << separator;

  b.dump();
}

TEST_F(Burton, mesh) {
  std::string separator;
  separator.insert(0,80,'=');
  separator.append("\n");

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "mesh" << endl;
  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "Vertices in mesh:" << endl;
  for(auto v : b.vertices()){
    CINCH_CAPTURE() << "----------- vertex id: " << v.id()
      << " with coordinates " << v->coordinates() << endl;
  } // for

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "Edges in mesh:" << endl;
  for(auto e : b.edges()){
    CINCH_CAPTURE() << "----------- edge id: " << e.id()
      << " with midpoint " << e->midpoint() << endl;
  } // for

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "Corners in mesh:" << endl;

  for(auto c : b.corners()) {
    CINCH_CAPTURE() << "----------- corner id: " << c.id() << endl;

    for(auto e: b.edges(c)) {
      CINCH_CAPTURE() << "       ++++ edge id: " << e.id() << endl;
    } // for
  } // for

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "Wedges in mesh:" << endl;

  for(auto w : b.wedges()) {
    CINCH_CAPTURE() << "----------- wedge id: " << w.id() << endl;

    for(auto c: b.cells(w)) {
      CINCH_CAPTURE() << "       ++++ cell id: " << c.id() << endl;
    } // for

    for(auto e: b.edges(w)) {
      CINCH_CAPTURE() << "       ++++ edge id: " << e.id() << endl;
    } // for

    for(auto v: b.vertices(w)) {
      CINCH_CAPTURE() << "       ++++ vertex id: " << v.id() << endl;
    } // for
  } // for

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "For each vertex:" << endl;
  for(auto v: b.vertices()) {
    CINCH_CAPTURE() << "^^^^^^^^Vertex id: " << v.id()
      << " with coordinates " << v->coordinates() << endl;
    CINCH_CAPTURE() << "    ----Wedges:" << endl;
    for(auto w: b.wedges(v)) {
      CINCH_CAPTURE() << "    ++++ wedge id: " << w.id() << endl;
    } // for
  } // for

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "For each cell:" << endl;
  for(auto c : b.cells()) {
    CINCH_CAPTURE() << "^^^^^^^^Cell id: " << c.id()
      << " with centroid " << c->centroid() << endl;

    CINCH_CAPTURE() << "    ----Edges:" << endl;
    for(auto e : b.edges(c)){
      CINCH_CAPTURE() << "    ++++ edge id: " << e.id()
        << " with midpoint " << e->midpoint() << endl;
    } // for

    CINCH_CAPTURE() << "    ----Wedges:" << endl;
    for(auto w : b.wedges(c)){
      CINCH_CAPTURE() << "    ++++ wedge id: " << w.id() << endl;
    } // for

    CINCH_CAPTURE() << "    ----Corners:" << endl;
    for(auto cnr : b.corners(c)){
      CINCH_CAPTURE() << "    ++++ corner id: " << cnr.id() << endl;
    } // for
  } // for

} // TEST_F

TEST_F(Burton, geometry) {
  std::string separator;
  separator.insert(0,80,'=');
  separator.append("\n");

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "geometry" << endl;
  CINCH_CAPTURE() << separator;
  for(auto c: b.cells()) {
    auto xc = c->centroid();
    CINCH_CAPTURE() << "---- cell id: " << c.id()
      << " with centroid " << xc << endl;
    for(auto v : b.vertices(c)){
      auto xv = v->coordinates();
      CINCH_CAPTURE() << "++++ vertex id: " << v.id()
        << " with coordinates " << xv << endl;
    } // for
  } // for

  CINCH_CAPTURE() << separator;
  for(auto v: b.vertices()) {
    auto xv = v->coordinates();
    CINCH_CAPTURE() << "^^^^ vertex id: " << v.id()
      << " with coordinates " << xv << endl;
    for(auto w: b.wedges(v)) {

      CINCH_CAPTURE() << "     ---- wedge id: " << w.id() << endl;

      // Why do we have to do .first()?
      auto c = b.cells(w).first();
      CINCH_CAPTURE() << "          ++++ cell id: " << c.id()
        << " with centroid " << c->centroid() << endl;

      auto e = b.edges(w).first();
      CINCH_CAPTURE() << "          ++++ edge id: " << e.id()
        << " with midpoint " << e->midpoint() << endl;

      CINCH_CAPTURE() << "          ++++ side_facet_normal: "
        << w->side_facet_normal() << endl;

      CINCH_CAPTURE() << "          ++++ cell_facet_normal: "
        << w->cell_facet_normal() << endl;

    } // for

  } // for

} // TEST_F

TEST_F(Burton, accessors) {
  std::string separator;
  separator.insert(0,80,'=');
  separator.append("\n");

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "accessors" << endl;
  CINCH_CAPTURE() << separator;

  register_state(b, "pressure", cells, real_t, persistent);
  register_state(b, "density", cells, real_t);
  register_state(b, "total energy", cells, real_t, persistent);
  register_state(b, "velocity", vertices, vector_t, persistent);
  register_state(b, "H", edges, vector_t);

  struct data_t {
    double x, y;
  };  
  register_global_state(b, "const", data_t);

  CINCH_CAPTURE() << "Accessing state with type real_t:" << endl;

  auto vr = access_type(b, real_t);
  for(auto v: vr) {
    CINCH_CAPTURE() << "\t" << v.label() << " has type real_t" << endl;
  } // for

  CINCH_CAPTURE() << endl;

  CINCH_CAPTURE() << "Accessing state with type data_t:" << endl;

  auto vd = access_type(b, data_t);
  for(auto v: vd) {
    CINCH_CAPTURE() << "\t" << v.label() << " has type data_t" << endl;
  } // for

  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "Accessing state with type real_t at cells:" << endl;

  auto va = access_type_if(b, real_t, is_at(cells));
  for(auto v: va) {
    CINCH_CAPTURE() << "\t" << v.label() <<
      " has type real_t and is at cells" << endl;
  } // for

  CINCH_CAPTURE() << endl;

  CINCH_CAPTURE() << "Accessing persistent state with type real_t at cells:"
    << endl;

  auto vp = access_type_if(b, real_t, is_persistent_at(cells));

  for(auto v: vp) {
    CINCH_CAPTURE() << "\t" << v.label() <<
      " has type real_t and is persistent at cells" << endl;
  } // for

  CINCH_CAPTURE() << endl;

  CINCH_CAPTURE() << "Accessing state with type vector_t at vertices:" << endl;

  auto vv = access_type_if(b, vector_t, is_at(vertices));
  for(auto v: vv) {
    CINCH_CAPTURE() << "\t" << v.label() <<
      " has type vector_t and is at vertices" << endl;
  } // for

  CINCH_CAPTURE() << endl;

  CINCH_CAPTURE()
    << "Accessing persistent state with type vector_t at vertices:" << endl;

  auto vpv = access_type_if(b, vector_t, is_persistent_at(vertices));
  for(auto v: vpv) {
    CINCH_CAPTURE() << "\t" << v.label() <<
      " has type vector_t and is persistent at vertices" << endl;
  } // for

  CINCH_CAPTURE() << endl;

  CINCH_CAPTURE() << "Accessing state with type vector_t at edges:" << endl;

  auto ve = access_type_if(b, vector_t, is_at(edges));
  for(auto v: ve) {
    CINCH_CAPTURE() << "\t" << v.label() <<
      " has type vector_t and is at edges" << endl;
  } // for

  CINCH_CAPTURE() << endl;

  CINCH_CAPTURE() << "Accessing state with type point_t at vertices:" << endl;

  auto pv = access_type_if(b, point_t, is_at(vertices));
  for(auto v: pv) {
    CINCH_CAPTURE() << "\t" << v.label() <<
      " has type point_t and is at vertices" << endl;
  } // for

  CINCH_CAPTURE() << endl;

  CINCH_CAPTURE() << "Accessing persistent state with type point_t at vertices:"
            << endl;

  auto ppv = access_type_if(b, point_t, is_persistent_at(vertices));
  for(auto v: ppv) {
    CINCH_CAPTURE() << "\t" << v.label() <<
      " has type point_t and is persistent at vertices" << endl;
  } // for

  CINCH_CAPTURE() << endl;

} // TEST_F

TEST_F(Burton, state) {
  std::string separator;
  separator.insert(0,80,'=');
  separator.append("\n");

  CINCH_CAPTURE() << separator;
  CINCH_CAPTURE() << "state" << endl;
  CINCH_CAPTURE() << separator;

  register_state(b, "pressure", cells, real_t, persistent);
  register_state(b, "velocity", vertices, vector_t, persistent);
  register_state(b, "H", edges, vector_t);
  register_state(b, "cornerdata", corners, int32_t);
  register_state(b, "wedgedata", wedges, bool);

  struct data_t {
    int x, y;
  };  
  register_global_state(b, "const", data_t);


  auto p = access_state(b, "pressure", real_t);
  auto velocity = access_state(b, "velocity", vector_t);
  auto H = access_state(b, "H", vector_t);
  auto cd = access_state(b, "cornerdata", int32_t);
  auto wd = access_state(b, "wedgedata", bool);
  auto c = access_global_state(b, "const", data_t);

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

  // corners
  ASSERT_EQ(16, b.num_corners());
  for (auto c: b.corners()) {
    cd[c] = c.id();
  } // for

  for (auto c: b.corners()) {
    ASSERT_EQ(c.id(), cd[c]);
  } // for

  ASSERT_EQ(32, b.num_wedges());
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

  // test global data
  c = { 1, 2 };
  ASSERT_EQ(1, c->x);  ASSERT_EQ(1, (*c).x);
  ASSERT_EQ(2, c->y);  ASSERT_EQ(2, (*c).y);

} // TEST_F

// A final dummy test to compare the blessed file and do CINCH_DUMP().
TEST_F(Burton, cinch_dump) {
  //CINCH_ASSERT(TRUE, CINCH_EQUAL_BLESSED("burton.blessed"));
  cout << CINCH_DUMP() << endl;
}

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
