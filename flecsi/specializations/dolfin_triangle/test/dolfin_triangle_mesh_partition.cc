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

#include "../dolfin_triangle_mesh.h"
#include "dolfin_triangle_fixture.h"

using namespace flecsi;
using namespace testing;

TEST_F(Dolfin_Triangle, number_of_vertex_partitions_should_be_2) {
  ASSERT_THAT(vertex_partitions, SizeIs(2));
}

TEST_F(Dolfin_Triangle, number_of_cell_partitions_should_be_2) {
  ASSERT_THAT(cell_partitions, SizeIs(2));
}
