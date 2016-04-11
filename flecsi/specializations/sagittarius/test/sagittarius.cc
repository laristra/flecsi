//
// Created by ollie on 4/5/16.
//

#include <cinchtest.h>
#include "../sagittarius_mesh.h"

using namespace flecsi;

static int quads[][4] = {
  {0, 1, 2, 3},
  {1, 4, 6, 2}
};

static int triangles[][3] = {
  {4, 5, 6},
  {2, 6, 7}
};

TEST(sagittarius__Test, topology) {
  sagittarius_mesh_t<sagittarius_types> constellation;

  // add vertices to the mesh
  std::vector<sagittarius_vertex_t *> vertices;
  for (size_t i = 0; i < 8; i++) {
    auto v = constellation.make<sagittarius_vertex_t>();
    constellation.add_entity<0, sagittarius_vertex_t::dimension>(v);
    vertices.push_back(v);
  }
  ASSERT_EQ(8, constellation.num_vertices());

  for (size_t i = 0; i < 2; i++) {
    auto cell = constellation.make<sagittarius_quad_t>();
    constellation.add_entity<2, 0>(cell);
    constellation.init_cell<0>(cell,
                               {vertices[quads[i][0]],
                                vertices[quads[i][1]],
                                vertices[quads[i][2]],
                                vertices[quads[i][3]]});
  }
  ASSERT_EQ(2, constellation.num_cells());

  for (size_t i = 0; i < 2; i++) {
    auto cell = constellation.make<sagittarius_triangle_t>();
    constellation.add_entity<2, 0>(cell);
    constellation.init_cell<0>(cell,
                               {vertices[triangles[i][0]],
                                vertices[triangles[i][1]],
                                vertices[triangles[i][2]]});
  }
  ASSERT_EQ(4, constellation.num_cells());

  constellation.init();

  ASSERT_EQ(11, constellation.num_edges());

  auto vs = constellation.vertices();
  CINCH_CAPTURE() << "vertex to vertex connectivities:\n";
  for (auto v0 : vs) {
    CINCH_CAPTURE() << v0.id() << ": ";
    for (auto v1 : constellation.vertices(v0)) {
      CINCH_CAPTURE() << v1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "vertex to edge connectivities:\n";
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto e: constellation.edges(v)) {
      CINCH_CAPTURE() << e.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "vertex to cell connectivities:\n";
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto c: constellation.cells(v)) {
      CINCH_CAPTURE() << c.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to vertex connectivities:\n";
  auto edges = constellation.edges();
  for (auto e: edges) {
    CINCH_CAPTURE() << e.id() << ": ";
    for (auto v: constellation.vertices(e)) {
      CINCH_CAPTURE() << v.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to edge connectivities:\n";
  for (auto e0: edges) {
    CINCH_CAPTURE() << e0.id() << ": ";
    for (auto e1: constellation.edges(e0)) {
      CINCH_CAPTURE() << e1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to cell connectivities:\n";
  for (auto e: edges) {
    CINCH_CAPTURE() << e.id() << ": ";
    for (auto c: constellation.cells(e)) {
      CINCH_CAPTURE() << c.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  auto cells = constellation.cells();
  CINCH_CAPTURE() << "cell to vertex connectivities:\n";
  for (auto c: cells) {
    CINCH_CAPTURE() << c.id() << ": ";
    for (auto v: constellation.vertices(c)) {
      CINCH_CAPTURE() << v.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to edge connectivities:\n";
  for (auto c: cells) {
    CINCH_CAPTURE() << c.id() << ": ";
    for (auto e: constellation.edges(c)) {
      CINCH_CAPTURE() << e.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to cell connectivities:\n";
  for (auto c0: cells) {
    CINCH_CAPTURE() << c0.id() << ": ";
    for (auto c1: constellation.cells(c0)) {
      CINCH_CAPTURE() << c1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_ASSERT(TRUE, CINCH_EQUAL_BLESSED("sagittarius.blessed"));
}