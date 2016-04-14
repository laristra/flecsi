/*~--------------------------------------------------------------------------~*
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
 *~--------------------------------------------------------------------------~*/

#ifndef FLECSI_SAGITTARIUS_MESH_H
#define FLECSI_SAGITTARIUS_MESH_H

#include <flecsi/mesh/mesh_topology.h>
#include "sagittarius_types.h"

namespace flecsi
{
template <typename mesh_type>
class sagittarius_mesh_t : public mesh_topology_t<mesh_type>
{
private:
  using super = mesh_topology_t<mesh_type>;

  const int quads[2][4] = {
    {0, 1, 2, 3},
    {1, 4, 6, 2}
  };

  const int triangles[2][3] = {
    {4, 5, 6},
    {2, 6, 7}
  };

  void init_connectivities() {
    // add vertices to the mesh
    std::vector<sagittarius_vertex_t *> vertices;
    for (size_t i = 0; i < 8; i++) {
      auto v = super::template make<sagittarius_vertex_t>();
      super::template add_entity<0, sagittarius_vertex_t::dimension>(v);
      vertices.push_back(v);
    }

    // add cells and cell to vertex connectivities to the mesh
    for (size_t i = 0; i < 2; i++) {
      auto cell = super::template make<sagittarius_quad_t>();
      super::template add_entity<2, 0>(cell);
      super::template init_cell<0>(cell,
                                 {vertices[quads[i][0]],
                                  vertices[quads[i][1]],
                                  vertices[quads[i][2]],
                                  vertices[quads[i][3]]});
    }

    // actually compute connectivities between entities
    for (size_t i = 0; i < 2; i++) {
      auto cell = super::template make<sagittarius_triangle_t>();
      super::template add_entity<2, 0>(cell);
      super::template init_cell<0>(cell,
                                 {vertices[triangles[i][0]],
                                  vertices[triangles[i][1]],
                                  vertices[triangles[i][2]]});
    }

    super::template init();
  }

public:
  sagittarius_mesh_t() {
    init_connectivities();
  }

  auto num_vertices() const {
    return super::template num_entities<0, 0>();
  }
  auto vertices() {
    return super::template entities<0, 0>();
  }
  template <typename Entity>
  auto vertices(Entity e) {
    return super::template entities<0, 0>(e);
  }

  auto num_edges() const {
    return super::template num_entities<1, 0>();
  }
  auto edges() {
    return super::template entities<1, 0>();
  }
  template <typename Entity>
  auto edges(Entity e) {
    return super::template entities<1, 0>(e);
  }

  auto num_cells() const {
    return super::template num_entities<2, 0>();
  }
  auto cells() {
    return super::template entities<2, 0>();
  }
  template <typename Entity>
  auto cells(Entity e) {
    return super::template entities<2, 0>(e);
  }
};
}
#endif //FLECSI_SAGITTARIUS_MESH_H
