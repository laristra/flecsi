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
#include "dolfin_triangle_fixture.h"

using namespace flecsi;
using namespace testing;

TEST_F(A_Dolfin_Triangle_Partitioned_In_Two, number_of_vertex_partitions_should_be_2) {
  ASSERT_THAT(vertex_partitions, SizeIs(2));
}

TEST_F(A_Dolfin_Triangle_Partitioned_In_Two, number_of_vertex_in_each_partion_should_be_5) {
  ASSERT_THAT(vertex_partitions[0].offset, SizeIs(5));
  ASSERT_THAT(vertex_partitions[1].offset, SizeIs(5));
}

TEST_F(A_Dolfin_Triangle_Partitioned_In_Two, number_of_cell_partitions_should_be_2) {
  ASSERT_THAT(cell_partitions, SizeIs(2));
}

TEST_F(A_Dolfin_Triangle_Partitioned_In_Two, number_of_cell_in_each_partition_should_be_5) {
  ASSERT_THAT(cell_partitions[0].offset, SizeIs(5));
  ASSERT_THAT(cell_partitions[1].offset, SizeIs(5));
}
