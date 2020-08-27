/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#define __FLECSI_PRIVATE__
#include "flecsi/execution.hh"
#include "flecsi/flog.hh"
#include "flecsi/topo/unstructured/closure_utils.hh"
#include "flecsi/topo/unstructured/test/test_definition.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;
using namespace flecsi::topo::unstructured_impl;

/*
  This test checks that the correct neighboring cells are calculated for
  each cell in a 4x4 mesh through vertex connections.
*/
int
cell_to_cell_thru_vertices() {
  UNIT {
    test_definition td;

#define neighbor_test(id, blessed)                                             \
  {                                                                            \
    auto neighbors = entity_neighbors<test_definition, 2, 2, 0>(td, id);       \
    ASSERT_EQ(neighbors, blessed);                                             \
  }

    neighbor_test(0, std::set<size_t>({1, 4, 5}));
    neighbor_test(1, std::set<size_t>({0, 2, 4, 5, 6}));
    neighbor_test(2, std::set<size_t>({1, 3, 5, 6, 7}));
    neighbor_test(3, std::set<size_t>({2, 6, 7}));
    neighbor_test(4, std::set<size_t>({0, 1, 5, 8, 9}));
    neighbor_test(5, std::set<size_t>({0, 1, 2, 4, 6, 8, 9, 10}));
    neighbor_test(6, std::set<size_t>({1, 2, 3, 5, 7, 9, 10, 11}));
    neighbor_test(7, std::set<size_t>({2, 3, 6, 10, 11}));
    neighbor_test(8, std::set<size_t>({4, 5, 9, 12, 13}));
    neighbor_test(9, std::set<size_t>({4, 5, 6, 8, 10, 12, 13, 14}));
    neighbor_test(10, std::set<size_t>({5, 6, 7, 9, 11, 13, 14, 15}));
    neighbor_test(11, std::set<size_t>({6, 7, 10, 14, 15}));
    neighbor_test(12, std::set<size_t>({8, 9, 13}));
    neighbor_test(13, std::set<size_t>({8, 9, 10, 12, 14}));
    neighbor_test(14, std::set<size_t>({9, 10, 11, 13, 15}));
    neighbor_test(15, std::set<size_t>({10, 11, 14}));
#undef neighbor_test
  };
}

/*
 This test checks that the correct neighboring cells are calculated for
 each cell in a 4x4 mesh through edge connections.
*/
int
cell_to_cell_thru_edges() {
  UNIT {
    test_definition td;

#define neighbor_test(id, blessed)                                             \
  {                                                                            \
    auto neighbors = entity_neighbors<test_definition, 2, 2, 1>(td, id);       \
    ASSERT_EQ(neighbors, blessed);                                             \
  }

    neighbor_test(0, std::set<size_t>({1, 4}));
    neighbor_test(1, std::set<size_t>({0, 2, 5}));
    neighbor_test(2, std::set<size_t>({1, 3, 6}));
    neighbor_test(3, std::set<size_t>({2, 7}));
    neighbor_test(4, std::set<size_t>({0, 5, 8}));
    neighbor_test(5, std::set<size_t>({1, 4, 6, 9}));
    neighbor_test(6, std::set<size_t>({2, 5, 7, 10}));
    neighbor_test(7, std::set<size_t>({3, 6, 11}));
    neighbor_test(8, std::set<size_t>({4, 9, 12}));
    neighbor_test(9, std::set<size_t>({5, 8, 10, 13}));
    neighbor_test(10, std::set<size_t>({6, 9, 11, 14}));
    neighbor_test(11, std::set<size_t>({7, 10, 15}));
    neighbor_test(12, std::set<size_t>({8, 13}));
    neighbor_test(13, std::set<size_t>({9, 12, 14}));
    neighbor_test(14, std::set<size_t>({10, 13, 15}));
    neighbor_test(15, std::set<size_t>({11, 14}));
#undef neighbor_test
  };
}

/*
 This test checks that the correct cell closure is created for
 a 4x4 mesh through vertex connections.
*/
int
cell_closure_thru_vertices() {
  UNIT {
    test_definition td;

    std::set<size_t> primary = {0, 1, 4, 5};
    auto closure = entity_neighbors<test_definition, 2, 2, 0>(td, primary);

    std::set<size_t> blessed = {0, 1, 2, 4, 5, 6, 8, 9, 10};
    ASSERT_EQ(blessed, closure);
  };
}

/*
 This test checks that the correct cell closure is created for
 a 4x4 mesh through edge connections.
*/
int
cell_closure_thru_edges() {
  UNIT {
    test_definition td;
    std::set<size_t> primary = {0, 1, 4, 5};
    auto closure = entity_neighbors<test_definition, 2, 2, 1>(td, primary);

    std::set<size_t> blessed = {0, 1, 2, 4, 5, 6, 8, 9};
    ASSERT_EQ(blessed, closure);
  };
}

/*
 This test checks that the correct set of cells is found that
 reference a given vertex id in a 4x4 mesh.
*/
int
vertex_referencers() {
  UNIT {
    test_definition td;

#define referencers_test(id, blessed)                                          \
  {                                                                            \
    auto referencers = entity_referencers<test_definition, 2, 0>(td, id);      \
    ASSERT_EQ(blessed, referencers);                                           \
  }

    referencers_test(0, std::set<size_t>({0}));
    referencers_test(1, std::set<size_t>({0, 1}));
    referencers_test(2, std::set<size_t>({1, 2}));
    referencers_test(3, std::set<size_t>({2, 3}));
    referencers_test(4, std::set<size_t>({3}));
    referencers_test(5, std::set<size_t>({0, 4}));
    referencers_test(6, std::set<size_t>({0, 1, 4, 5}));
    referencers_test(7, std::set<size_t>({1, 2, 5, 6}));
    referencers_test(8, std::set<size_t>({2, 3, 6, 7}));
    referencers_test(9, std::set<size_t>({3, 7}));
    referencers_test(10, std::set<size_t>({4, 8}));
    referencers_test(11, std::set<size_t>({4, 5, 8, 9}));
    referencers_test(12, std::set<size_t>({5, 6, 9, 10}));
    referencers_test(13, std::set<size_t>({6, 7, 10, 11}));
    referencers_test(14, std::set<size_t>({7, 11}));
    referencers_test(15, std::set<size_t>({8, 12}));
    referencers_test(16, std::set<size_t>({8, 9, 12, 13}));
    referencers_test(17, std::set<size_t>({9, 10, 13, 14}));
    referencers_test(18, std::set<size_t>({10, 11, 14, 15}));
    referencers_test(19, std::set<size_t>({11, 15}));
    referencers_test(20, std::set<size_t>({12}));
    referencers_test(21, std::set<size_t>({12, 13}));
    referencers_test(22, std::set<size_t>({13, 14}));
    referencers_test(23, std::set<size_t>({14, 15}));
    referencers_test(24, std::set<size_t>({15}));

#undef referencers_test
  };
}

/* This test checks that the correct set of cells is found that
   reference a given vertex id in a 4x4 mesh.
*/
int
vertex_closure() {
  UNIT {
    test_definition td;

    std::set<size_t> primary = {0, 1, 4, 5};
    auto closure = entity_closure<test_definition, 2, 0>(td, primary);

    std::set<size_t> blessed = {0, 1, 2, 5, 6, 7, 10, 11, 12};
    ASSERT_EQ(blessed, closure);
  };
}

int
closure_driver() {
  UNIT {
    EXPECT_EQ(test<cell_to_cell_thru_vertices>(), 0);
    EXPECT_EQ(test<cell_to_cell_thru_edges>(), 0);
    EXPECT_EQ(test<cell_closure_thru_vertices>(), 0);
    EXPECT_EQ(test<cell_closure_thru_edges>(), 0);
    EXPECT_EQ(test<vertex_referencers>(), 0);
    EXPECT_EQ(test<vertex_closure>(), 0);
  };
}

flecsi::unit::driver<closure_driver> driver;
