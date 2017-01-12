/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/topology/graph_utils.h"
#include "flecsi/topology/test/test_definition.h"

clog_register_tag(neighbors);
clog_register_tag(referencers);

// This test checks that the correct neighboring cells are calculated for
// each cell in a 4x4 mesh through vertex connections.
TEST(graph, cell_to_cell_thru_vertices) {

  flecsi::topology::test_definition_t td;

  auto neighbors = flecsi::topology::entity_neighbors<2,2,0>(td, 0);

#define neighbor_test(id, set)                                                 \
  {                                                                            \
  auto neighbors = flecsi::topology::entity_neighbors<2,2,0>(td, id);          \
  clog_container(info, "neighbors " << id, neighbors, clog::space);            \
  } // scope

  {
  clog_tag_guard(neighbors);

  neighbor_test(0, std::set<size_t>({ 1, 4, 5 }));
  neighbor_test(1, std::set<size_t>({ 0, 2, 4, 5, 6 }));
  neighbor_test(2, std::set<size_t>({ 1, 3, 5, 6, 7 }));
  neighbor_test(3, std::set<size_t>({ 2, 6, 7 }));
  neighbor_test(4, std::set<size_t>({ 0, 1, 5, 8, 9 }));
  neighbor_test(5, std::set<size_t>({ 0, 1, 2, 4, 6, 8, 9, 10 }));
  neighbor_test(6, std::set<size_t>({ 1, 2, 3, 5, 7, 9, 10, 11 }));
  neighbor_test(7, std::set<size_t>({ 2, 3, 6, 10, 11 }));
  neighbor_test(8, std::set<size_t>({ 4, 5, 9, 12, 13 }));
  neighbor_test(9, std::set<size_t>({ 4, 5, 6, 8, 10, 12, 13, 14 }));
  neighbor_test(10, std::set<size_t>({ 5, 6, 7, 9, 11, 13, 14, 15 }));
  neighbor_test(11, std::set<size_t>({ 6, 7, 10, 14, 15 }));
  neighbor_test(12, std::set<size_t>({ 8, 9, 13 }));
  neighbor_test(13, std::set<size_t>({ 8, 9, 10, 12, 14 }));
  neighbor_test(14, std::set<size_t>({ 9, 10, 11, 13, 15 }));
  neighbor_test(15, std::set<size_t>({ 10, 11, 14 }));
  } // guard

#undef neighbor_test

} // TEST

// This test checks that the correct neighboring cells are calculated for
// each cell in a 4x4 mesh through edge connections.
TEST(graph, cell_to_cell_thru_edges) {

  flecsi::topology::test_definition_t td;

#define neighbor_test(id, set)                                                 \
  {                                                                            \
  auto neighbors = flecsi::topology::entity_neighbors<2,2,1>(td, id);          \
  clog_container(info, "neighbors " << id, neighbors, clog::space);            \
  CINCH_ASSERT(EQ, set, neighbors);                                            \
  } // scope

  {
  clog_tag_guard(neighbors);

  neighbor_test(0, std::set<size_t>({ 1, 4 }));
  neighbor_test(1, std::set<size_t>({ 0, 2, 5 }));
  neighbor_test(2, std::set<size_t>({ 1, 3, 6 }));
  neighbor_test(3, std::set<size_t>({ 2, 7 }));
  neighbor_test(4, std::set<size_t>({ 0, 5, 8 }));
  neighbor_test(5, std::set<size_t>({ 1, 4, 6, 9 }));
  neighbor_test(6, std::set<size_t>({ 2, 5, 7, 10 }));
  neighbor_test(7, std::set<size_t>({ 3, 6, 11 }));
  neighbor_test(8, std::set<size_t>({ 4, 9, 12 }));
  neighbor_test(9, std::set<size_t>({ 5, 8, 10, 13 }));
  neighbor_test(10, std::set<size_t>({ 6, 9, 11, 14 }));
  neighbor_test(11, std::set<size_t>({ 7, 10, 15 }));
  neighbor_test(12, std::set<size_t>({ 8, 13 }));
  neighbor_test(13, std::set<size_t>({ 9, 12, 14 }));
  neighbor_test(14, std::set<size_t>({ 10, 13, 15 }));
  neighbor_test(15, std::set<size_t>({ 11, 14 }));
  } // guard

#undef neighbor_test

} // TEST

// This test checks that the correct cell closure is created for
// a 4x4 mesh through vertex connections.
TEST(graph, cell_closure_thru_vertices) {

  flecsi::topology::test_definition_t td;

  std::set<size_t> primary = { 0, 1, 4, 5 };
  auto closure = flecsi::topology::entity_closure<2,2,0>(td, primary);

  clog_container(info, "closure ", closure, clog::space);

  std::set<size_t> compare = { 0, 1, 2, 4, 5, 6, 8, 9, 10 };
  CINCH_ASSERT(EQ, compare, closure);

} // TEST

// This test checks that the correct cell closure is created for
// a 4x4 mesh through edge connections.
TEST(graph, cell_closure_thru_edges) {

  flecsi::topology::test_definition_t td;

  std::set<size_t> primary = { 0, 1, 4, 5 };
  auto closure = flecsi::topology::entity_closure<2,2,1>(td, primary);

  clog_container(info, "closure ", closure, clog::space);

  std::set<size_t> compare = { 0, 1, 2, 4, 5, 6, 8, 9 };
  CINCH_ASSERT(EQ, compare, closure);

} // TEST

// This test checks that the correct set of cells is found that
// reference a given vertex id in a 4x4 mesh.
TEST(graph, vertex_referencers) {

  flecsi::topology::test_definition_t td;

#define referencers_test(id, set)                                              \
  {                                                                            \
  auto referencers = flecsi::topology::vertex_referencers<2>(td, id);          \
  clog_container(info, "referencers " << id, referencers, clog::space);        \
  CINCH_ASSERT(EQ, set, referencers);                                          \
  } // scope

  {
  clog_tag_guard(referencers);

  referencers_test(0, std::set<size_t>({ 0 }));
  referencers_test(1, std::set<size_t>({ 0, 1 }));
  referencers_test(2, std::set<size_t>({ 1, 2 }));
  referencers_test(3, std::set<size_t>({ 2, 3 }));
  referencers_test(4, std::set<size_t>({ 3 }));
  referencers_test(5, std::set<size_t>({ 0, 4 }));
  referencers_test(6, std::set<size_t>({ 0, 1, 4, 5 }));
  referencers_test(7, std::set<size_t>({ 1, 2, 5, 6 }));
  referencers_test(8, std::set<size_t>({ 2, 3, 6, 7 }));
  referencers_test(9, std::set<size_t>({ 3, 7 }));
  referencers_test(10, std::set<size_t>({ 4, 8 }));
  referencers_test(11, std::set<size_t>({ 4, 5, 8, 9 }));
  referencers_test(12, std::set<size_t>({ 5, 6, 9, 10 }));
  referencers_test(13, std::set<size_t>({ 6, 7, 10, 11 }));
  referencers_test(14, std::set<size_t>({ 7, 11 }));
  referencers_test(15, std::set<size_t>({ 8, 12 }));
  referencers_test(16, std::set<size_t>({ 8, 9, 12, 13 }));
  referencers_test(17, std::set<size_t>({ 9, 10, 13, 14 }));
  referencers_test(18, std::set<size_t>({ 10, 11, 14, 15 }));
  referencers_test(19, std::set<size_t>({ 11, 15 }));
  referencers_test(20, std::set<size_t>({ 12 }));
  referencers_test(21, std::set<size_t>({ 12, 13 }));
  referencers_test(22, std::set<size_t>({ 13, 14 }));
  referencers_test(23, std::set<size_t>({ 14, 15 }));
  referencers_test(24, std::set<size_t>({ 15 }));
  } // guard

#undef referencers_test

} // TEST

// This test checks that the correct set of cells is found that
// reference a given vertex id in a 4x4 mesh.
TEST(graph, vertex_closure) {

  flecsi::topology::test_definition_t td;

  std::set<size_t> primary = { 0, 1, 4, 5 };
  auto closure = flecsi::topology::vertex_closure<2>(td, primary);

  clog_container(info, "closure ", closure, clog::space);

  std::set<size_t> compare = { 0, 1, 2, 5, 6, 7, 10, 11, 12 };
  CINCH_ASSERT(EQ, compare, closure);

} // TEST

/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << std::endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 *  CINCH_ASSERT(ASSERTION, ...) : Call Google test macro and automatically
 *                                 dump captured output (from CINCH_CAPTURE)
 *                                 on failure.
 *
 *  CINCH_EXPECT(ASSERTION, ...) : Call Google test macro and automatically
 *                                 dump captured output (from CINCH_CAPTURE)
 *                                 on failure.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
