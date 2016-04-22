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

class Cell2VertexConnectivity : public Test {
protected:
  dolfin_triangle_mesh_t<dolfin_triangle_types_t> dolfin;

  virtual void SetUp() override {
    conn = dolfin.get_connectivity(0, 2, 0);
  }
  connectivity_t conn;
};

TEST_F(Cell2VertexConnectivity, from_size_equals_to_number_of_cells) {
  ASSERT_THAT(conn.from_size(), Eq(dolfin.num_cells()));
}

TEST_F(Cell2VertexConnectivity, to_size_euqals_to_number_of_cell_times_3) {
  ASSERT_THAT(conn.to_size(), Eq(dolfin.num_cells()*3));
}

TEST_F(Cell2VertexConnectivity,
       from_index_is_an_exclusive_scan_of_number_of_vertices_per_cell) {
  std::vector<size_t> vertices_per_cell(10, 3);
  std::vector<size_t> exscan(11);
  std::partial_sum(vertices_per_cell.begin(), vertices_per_cell.end(),
                   exscan.begin()+1);

  ASSERT_THAT(conn.get_from_index_vec(),
              ElementsAreArray(exscan));
}
