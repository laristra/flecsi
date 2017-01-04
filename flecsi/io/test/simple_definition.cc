/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/io/simple_definition.h"
#include "flecsi/topology/graph_utils.h"
#include "flecsi/utils/set_utils.h"

TEST(simple_definition, simple) {

  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");

  // All values are hard-coded for the test and depend on the
  // input file being correct
  const size_t M = 8; // rows
  const size_t N = 8; // columns

  // 8x8 = 64 cells
  CINCH_ASSERT(TRUE, sd.num_entities(0) == 81);
  CINCH_ASSERT(TRUE, sd.num_entities(2) == 64);

  for(size_t c(0); c<sd.num_entities(2); ++c) {
    size_t row = c/M;
    size_t col = c%M;

    size_t r0 = col + row*(M+1);
    size_t r1 = r0 + M+1;

    auto ids = sd.vertices(2, c);

    CINCH_ASSERT(EQ, ids[0], r0);
    CINCH_ASSERT(EQ, ids[1], (r0+1));
    CINCH_ASSERT(EQ, ids[2], (r1+1));
    CINCH_ASSERT(EQ, ids[3], r1);
  } // for

  double xinc = 1.0/N;
  double yinc = 1.0/M;

  for(size_t v(0); v<sd.num_entities(2); ++v) {
    size_t row = v/(M+1);
    size_t col = v%(M+1);

    auto coords = sd.vertex(v);

    CINCH_ASSERT(EQ, coords[0], col*xinc);
    CINCH_ASSERT(EQ, coords[1], row*yinc);
  } // for

} // TEST

TEST(simple_definition, neighbors) {

  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");

  // Primary partititon
  std::set<size_t> partition = { 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19 };

  // The closure captures any cell that is adjacent to a cell in the
  // set of indices passed to the method. The closure includes the
  // initial set of indices.
  auto closure = flecsi::topology::entity_closure<2,2,1>(sd, partition);

  CINCH_ASSERT(EQ, closure, std::set<size_t>({0, 1, 2, 3, 4, 8, 9, 10, 11,
                                             12, 16, 17, 18, 19, 20, 24,
                                             25, 26, 27}));

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nn = flecsi::utils::set_difference(closure, partition);

  CINCH_ASSERT(EQ, nn, std::set<size_t>({4, 12, 20, 24, 25, 26, 27}));

  // The closure of the nearest neighbors intersected with
  // the initial indeces gives the shared indices. This is similar to
  // the preimage of the nearest neighbors.
  auto nnclosure = flecsi::topology::entity_closure<2,2,1>(sd, nn);
  auto shared = flecsi::utils::set_intersection(nnclosure, partition);

  CINCH_ASSERT(EQ, shared, std::set<size_t>({3, 11, 16, 17, 18, 19}));

  // One can iteratively add halos of nearest neighbors, e.g.,
  // here we add the next nearest neighbors.
  auto nnn = flecsi::utils::set_difference(nnclosure, closure);
  CINCH_ASSERT(EQ, nnn, std::set<size_t>({5, 13, 21, 28, 32, 33, 34, 35}));

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
