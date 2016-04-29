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

#ifndef FLECSI_SAGITTARIUS_FIXTURE_H
#define FLECSI_SAGITTARIUS_FIXTURE_H

namespace flecsi {

class A_Sagittarius_Mesh : public ::testing::Test {
protected:
  sagittarius_mesh_t <sagittarius_types> constellation;
};

class A_Sagittarius_Mesh_Partitioned_In_Two : public ::testing::Test {
protected:
  sagittarius_mesh_t <sagittarius_types> constellation;

  virtual void SetUp() override {
    // convert and divide vertex to vertex and cell to cell connectivities into
    // two equal partitions in the form of Distributed CSR format as in ParMetis
    // manual.
    constellation.compute_graph_partition(0, 0, vertex_sizes, vertex_partitions);
    constellation.compute_graph_partition(0, 2, cell_sizes, cell_partitions);
  }

  virtual void TearDown() override { }

  std::vector<size_t> vertex_sizes = {4, 4};
  std::vector<mesh_graph_partition<size_t>> vertex_partitions;
  std::vector<size_t> cell_sizes = {2, 2};
  std::vector<mesh_graph_partition<size_t>> cell_partitions;
};
}
#endif //FLECSI_SAGITTARIUS_FIXTURE_H
