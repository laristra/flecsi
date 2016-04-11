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

#ifndef FLECSI_DOLFIN_TRIANGLE_FIXTURE_H
#define FLECSI_DOLFIN_TRIANGLE_FIXTURE_H

namespace flecsi {

class Dolfin_Triangle : public ::testing::Test {
protected:
  dolfin_triangle_mesh_t<dolfin_triangle_types_t> dolfin;

  virtual void SetUp() {
    // add vertices to the mesh
    std::vector<dolfin_vertex_t *> vertices;
    for (size_t i = 0; i < 10; i++) {
      auto v = dolfin.make<dolfin_vertex_t>();
      dolfin.add_entity<0, dolfin_vertex_t::dimension>(v);
      vertices.push_back(v);
    }

    // add cells and cell to vertex connectivities to the mesh
    for (size_t i = 0; i < 10; i++) {
      auto cell = dolfin.make<dolfin_cell_t>();
      dolfin.add_entity<2, 0>(cell);
      dolfin.init_cell<0>(cell,
                          {vertices[cell_to_vertices[i][0]],
                           vertices[cell_to_vertices[i][1]],
                           vertices[cell_to_vertices[i][2]]});
    }

    // actually compute connectivities between entities
    dolfin.init();

    // convert vertex to vertex and cell to cell connectivities into
    // Distributed CSR format suitable for Metis
    dolfin.compute_graph_partition(0, 0, vertex_sizes, vertex_partitions);
    dolfin.compute_graph_partition(0, 2, cell_sizes, cell_partitions);
  }

  const int cell_to_vertices[10][3] = {
    {0, 1, 8}, {1, 2, 8}, {2, 3, 8}, {3, 9, 8}, {3, 4, 9},
    {4, 5, 9}, {5, 6, 9}, {6, 7, 9}, {7, 8, 9}, {7, 0, 8},
  };

  std::vector<size_t> vertex_sizes = {5, 5};
  std::vector<mesh_graph_partition<size_t>> vertex_partitions;
  std::vector<size_t> cell_sizes = {5, 5};
  std::vector<mesh_graph_partition<size_t>> cell_partitions;
};
}
#endif //FLECSI_DOLFIN_TRIANGLE_FIXTURE_H
