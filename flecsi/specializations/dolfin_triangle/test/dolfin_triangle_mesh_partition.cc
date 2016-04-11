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

TEST_F(Dolfin_Triangle, number_of_vertex_partitions_should_be_2) {
  ASSERT_EQ(2, vertex_partitions.size());
}

TEST_F(Dolfin_Triangle, number_of_cell_partitions_should_be_2) {
  ASSERT_EQ(2, cell_partitions.size());
}
