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
#include "../sagittarius_mesh.h"

using namespace flecsi;

static const int quads[][4] = {
  {0, 1, 2, 3},
  {1, 4, 6, 2}
};

static const int triangles[][3] = {
  {4, 5, 6},
  {2, 6, 7}
};

class Sagittarius : public ::testing::Test {
protected:
  sagittarius_mesh_t<sagittarius_types> constellation;

  virtual void SetUp() override  {
    // add vertices to the mesh
    std::vector<sagittarius_vertex_t *> vertices;
    for (size_t i = 0; i < 8; i++) {
      auto v = constellation.make<sagittarius_vertex_t>();
      constellation.add_entity<0, sagittarius_vertex_t::dimension>(v);
      vertices.push_back(v);
    }

    for (size_t i = 0; i < 2; i++) {
      auto cell = constellation.make<sagittarius_quad_t>();
      constellation.add_entity<2, 0>(cell);
      constellation.init_cell<0>(cell,
                                 {vertices[quads[i][0]],
                                  vertices[quads[i][1]],
                                  vertices[quads[i][2]],
                                  vertices[quads[i][3]]});
    }

    for (size_t i = 0; i < 2; i++) {
      auto cell = constellation.make<sagittarius_triangle_t>();
      constellation.add_entity<2, 0>(cell);
      constellation.init_cell<0>(cell,
                                 {vertices[triangles[i][0]],
                                  vertices[triangles[i][1]],
                                  vertices[triangles[i][2]]});
    }

    constellation.init();


    constellation.compute_graph_partition(0, 0, sizes, partitions);
  }

  virtual void TearDown() override  {}

  std::vector<size_t> sizes = {4, 4};
  std::vector<mesh_graph_partition<size_t>> partitions;
};

TEST_F(Sagittarius, number_of_vertices_should_be_8) {
  ASSERT_EQ(8, constellation.num_vertices());
}

TEST_F(Sagittarius, number_partitions_should_be_2) {
  ASSERT_EQ(2, partitions.size());
}

TEST_F(Sagittarius, number_of_vertices_in_each_partition_should_be_4) {
  ASSERT_EQ(4, partitions[0].offset.size());
  ASSERT_EQ(4, partitions[1].offset.size());
}

//TEST_F(Sagittarius, )
TEST_F(Sagittarius, dump) {
  for (auto partition : partitions) {
    std::cout << "offset: ";
    for (auto offset : partition.offset) {
      std::cout << offset << " ";
    }
    std::cout << std::endl;
    std::cout << "index: ";
    for (auto i : partition.index) {
      std::cout << i << " ";
    }
    std::cout << std::endl;
  }
  //ASSERT_EQ(0, partitions[1].partition.size());
}