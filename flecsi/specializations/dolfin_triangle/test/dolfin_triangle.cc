//
// Created by ollie on 3/31/16.
//

#include <cinchtest.h>
#include "../dolfin_triangle_mesh.h"

using namespace flecsi;

TEST(dolfin_triangle, initialization) {
  dolfin_triangle_mesh_t<dolfin_triangle_types_t> dolfin;

  // there are no vertex, edge or cells at the beginning
  ASSERT_EQ(0, dolfin.num_entities(0, 0));
  ASSERT_EQ(0, dolfin.num_entities(0, 1));
  ASSERT_EQ(0, dolfin.num_entities(0, 2));

  // add vertices to the mesh
  std::vector<dolfin_vertex_t *> vertices;
  for (size_t i = 0; i < 10; i++) {
    auto v = dolfin.make<dolfin_vertex_t>();
    dolfin.add_entity<0, 0>(v);
    vertices.push_back(v);
  }
  ASSERT_EQ(10, dolfin.num_entities(0, 0));
  
  //dolfin.init();
}