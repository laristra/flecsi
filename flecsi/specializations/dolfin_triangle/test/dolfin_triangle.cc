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
  ASSERT_EQ(10, dolfin.num_entities(0, 0));

  // add cells to the mesh
  for (size_t i = 0; i < 10; i++) {
    auto cell = dolfin.make<dolfin_cell_t>();
    dolfin.add_entity<2, 0>(cell);
    dolfin.init_cell<0>(cell,
                        {vertices[cell_to_vertices[i][0]],
                         vertices[cell_to_vertices[i][1]],
                         vertices[cell_to_vertices[i][2]]});
  }
  ASSERT_EQ(10, dolfin.num_entities(0, 2));

  // actually compute connectivities between entities
  dolfin.init();

  auto con = dolfin.get_connectivity(0, 0, 2);
  con.dump();
}