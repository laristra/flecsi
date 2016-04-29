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

#ifndef FLECSI_DOLFIN_TRIANGLE_MESH_H
#define FLECSI_DOLFIN_TRIANGLE_MESH_H

#include "flecsi/mesh/mesh_topology.h"
#include "dolfin_triangle_types.h"

/*!
 * \file dolfin_mesh.h
 * \authors ollie
 * \date Initial file creation: Mar. 31, 2016
 */

namespace flecsi {

/*!
 * \class dolfin_triangle_mesh dolfin_triangle_mesh.h
 * \brief example mesh in Figure 1 and Figure 2 of the DOLFIN paper.
 */
template<typename mesh_type>
class dolfin_triangle_mesh_t : public mesh_topology_t<mesh_type> {
private:
  using super = mesh_topology_t<mesh_type>;

  const int cell_to_vertices[10][3] = {
    {0, 1, 8}, {1, 2, 8}, {2, 3, 8}, {3, 9, 8}, {3, 4, 9},
    {4, 5, 9}, {5, 6, 9}, {6, 7, 9}, {7, 8, 9}, {7, 0, 8},
  };

public:
  dolfin_triangle_mesh_t() {
    // add vertices to the mesh
    std::vector<dolfin_vertex_t *> vertices;
    for (size_t i = 0; i < 10; i++) {
      auto v = super::template make<dolfin_vertex_t>();
      super::template add_entity<0, dolfin_vertex_t::dimension>(v);
      vertices.push_back(v);
    }

    // add cells and cell to vertex connectivities to the mesh
    for (size_t i = 0; i < 10; i++) {
      auto cell = super::template make<dolfin_cell_t>();
      super::template add_entity<2, 0>(cell);
      super::template init_cell<0>(cell,
                                   {vertices[cell_to_vertices[i][0]],
                                    vertices[cell_to_vertices[i][1]],
                                    vertices[cell_to_vertices[i][2]]});
    }

    // actually compute connectivities between entities
    super::template init();
  }

  auto num_vertices() const {
    return super::template num_entities<0, 0>();
  }

  auto vertices() {
    return super::template entities<0, 0>();
  }

  template<typename Entity>
  auto vertices(Entity e) {
    return super::template entities<0, 0>(e);
  }

  auto num_edges() const {
    return super::template num_entities<1, 0>();
  }

  auto edges() {
    return super::template entities<1, 0>();
  }

  template<typename Entity>
  auto edges(Entity e) { return super::template entities<1, 0>(e); }

  auto num_cells() const {
    return super::template num_entities<2, 0>();
  }

  auto cells() { return super::template entities<2, 0>(); }

  template<typename Entity>
  auto cells(Entity e) { return super::template entities<2, 0>(e); }
};

}
#endif //FLECSI_DOLFIN_TRIANGLE_MESH_H
