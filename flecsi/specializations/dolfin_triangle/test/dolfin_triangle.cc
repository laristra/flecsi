//
// Created by ollie on 3/31/16.
//

#include <cinchtest.h>
#include "../dolfin_triangle_mesh.h"

using namespace flecsi;

static int cell_to_vertices[][3] = {
  {0, 1, 8}, {1, 2, 8}, {2, 3, 8}, {3, 9, 8}, {3, 4, 9},
  {4, 5, 9}, {5, 6, 9}, {6, 7, 9}, {7, 8, 9}, {7, 0, 8},
};


TEST(dolfin_triangle, initialization) {
  dolfin_triangle_mesh_t<dolfin_triangle_types_t> dolfin;

  // there are no vertex, edge or cells at the beginning
  ASSERT_EQ(0, dolfin.num_entities(0, dolfin_vertex_t::dimension));
  ASSERT_EQ(0, dolfin.num_entities(0, dolfin_edge_t::dimension));
  ASSERT_EQ(0, dolfin.num_entities(0, dolfin_cell_t::dimension));

  // add vertices to the mesh
  std::vector<dolfin_vertex_t *> vertices;
  for (size_t i = 0; i < 10; i++) {
    auto v = dolfin.make<dolfin_vertex_t>();
    dolfin.add_entity<0, dolfin_vertex_t::dimension>(v);
    vertices.push_back(v);
  }
  // There are 10 vertices.
  ASSERT_EQ(10, dolfin.num_vertices());

  // add cells and cell to vertex connectivities to the mesh
  for (size_t i = 0; i < 10; i++) {
    auto cell = dolfin.make<dolfin_cell_t>();
    dolfin.add_entity<2, 0>(cell);
    dolfin.init_cell<0>(cell,
                        {vertices[cell_to_vertices[i][0]],
                         vertices[cell_to_vertices[i][1]],
                         vertices[cell_to_vertices[i][2]]});
  }
  // There are 10 cells.
  ASSERT_EQ(10, dolfin.num_cells());

  // actually compute connectivities between entities
  dolfin.init();

  // this should create 19 edges from cells->vertices
  ASSERT_EQ(19, dolfin.num_edges());

  CINCH_CAPTURE() << "vertex to vertex connectivities:\n";
  auto vs = dolfin.vertices();
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto v1 : dolfin.vertices(v)) {
      CINCH_CAPTURE() << v1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "vertex to edge connectivities:\n";
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto e: dolfin.edges(v)) {
      CINCH_CAPTURE() << e.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "vertex to cell connectivities:\n";
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto c: dolfin.cells(v)) {
      CINCH_CAPTURE() << c.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to vertex connectivities:\n";
  auto edges = dolfin.edges();
  for (auto e : edges) {
    CINCH_CAPTURE() << e.id() << ": ";
    for (auto v: dolfin.vertices(e)) {
      CINCH_CAPTURE() << v.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to edge connectivities:\n";
  for (auto e0: edges) {
    CINCH_CAPTURE() << e0.id() << ": ";
    for (auto e1 : dolfin.edges(e0)) {
      CINCH_CAPTURE() << e1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to cell connectivities:\n";
  for (auto e: edges) {
    CINCH_CAPTURE() << e.id() << ": ";
    for (auto c: dolfin.cells(e)) {
      CINCH_CAPTURE() << c.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to vertex connectivities:\n";
  auto cells = dolfin.cells();
  for (auto c: cells) {
    CINCH_CAPTURE() << c.id() << ": ";
    for (auto v: dolfin.vertices(c)) {
      CINCH_CAPTURE() << v.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to edge connectivities:\n";
  for (auto c: cells) {
    CINCH_CAPTURE() << c.id() << ": ";
    for (auto e: dolfin.edges(c)) {
      CINCH_CAPTURE() << e.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to cell connectivities:\n";
  for (auto c0: cells) {
    CINCH_CAPTURE() << c0.id() << ": ";
    for (auto c1: dolfin.cells(c0)) {
      CINCH_CAPTURE() << c1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_ASSERT(TRUE, CINCH_EQUAL_BLESSED("dolfin_triangle.blessed"));
}