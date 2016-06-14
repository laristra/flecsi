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

#include "../dolfin_triangle_mesh.h"

using namespace flecsi;
using namespace testing;

class A_Dolfin_Triangle_Partitioned_In_Two : public ::testing::Test {
protected:
  dolfin_triangle_mesh_t<dolfin_triangle_types_t> dolfin;

  virtual void SetUp() {
    // convert and divide vertex to vertex and cell to cell connectivities into
    // two equal partitions in the form of Distributed CSR format as in ParMetis
    // manual.
    dolfin.compute_graph_partition(0, 0, vertex_sizes, vertex_partitions);
    dolfin.compute_graph_partition(0, 2, cell_sizes, cell_partitions);
  }

  std::vector<size_t> vertex_sizes = {5, 5};
  std::vector<mesh_graph_partition<size_t>> vertex_partitions;
  std::vector<size_t> cell_sizes = {5, 5};
  std::vector<mesh_graph_partition<size_t>> cell_partitions;
};

TEST_F(A_Dolfin_Triangle_Partitioned_In_Two,
       number_of_vertex_partitions_should_be_2) {
  ASSERT_THAT(vertex_partitions, SizeIs(2));
}

TEST_F(A_Dolfin_Triangle_Partitioned_In_Two,
       number_of_vertex_in_each_partion_should_be_5) {
  ASSERT_THAT(vertex_partitions[0].offset, SizeIs(5));
  ASSERT_THAT(vertex_partitions[1].offset, SizeIs(5));
}

TEST_F(A_Dolfin_Triangle_Partitioned_In_Two,
       number_of_cell_partitions_should_be_2) {
  ASSERT_THAT(cell_partitions, SizeIs(2));
}

TEST_F(A_Dolfin_Triangle_Partitioned_In_Two,
       number_of_cell_in_each_partition_should_be_5) {
  ASSERT_THAT(cell_partitions[0].offset, SizeIs(5));
  ASSERT_THAT(cell_partitions[1].offset, SizeIs(5));
}
