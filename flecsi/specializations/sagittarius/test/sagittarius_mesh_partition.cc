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
#include <gmock/gmock.h>

#include "../sagittarius_mesh.h"
#include "sagittarius_fixture.h"

using namespace flecsi;

TEST_F(Sagittarius, number_of_vertex_partitions_should_be_2) {
  ASSERT_EQ(2, vertex_partitions.size());
}

TEST_F(Sagittarius, number_of_vertices_in_each_partition_should_be_4) {
  ASSERT_EQ(4, vertex_partitions[0].offset.size());
  ASSERT_EQ(4, vertex_partitions[1].offset.size());

  ASSERT_THAT(vertex_partitions[0].partition, ::testing::ElementsAre(0, 4, 4));
  ASSERT_THAT(vertex_partitions[1].partition, ::testing::ElementsAre(0, 4, 4));
}

TEST_F(Sagittarius, degree_of_vertices) {
  // FIXME: this test is still not intuitive enough.
  auto diff0 = std::vector<size_t>(vertex_partitions[0].offset.size());
  std::adjacent_difference(vertex_partitions[0].offset.begin(),
                           vertex_partitions[0].offset.end(),
                           diff0.begin());
  ASSERT_THAT(diff0, ::testing::ElementsAre(0, 2, 3, 4));

  auto diff1 = std::vector<size_t>(vertex_partitions[1].offset.size());
  std::adjacent_difference(vertex_partitions[1].offset.begin(),
                           vertex_partitions[1].offset.end(),
                           diff1.begin());
  ASSERT_THAT(diff1, ::testing::ElementsAre(0, 3, 2, 4));
}

TEST_F(Sagittarius, number_of_cell_partitions_should_be_2) {
  ASSERT_EQ(2, cell_partitions.size());
}

TEST_F(Sagittarius, number_of_cells_in_each_partition_should_be_2) {
  ASSERT_EQ(2, cell_partitions[0].offset.size());
  ASSERT_EQ(2, cell_partitions[1].offset.size());
}