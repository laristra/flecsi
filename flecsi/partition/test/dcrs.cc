/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>
#include <mpi.h>

#include "flecsi/io/simple_definition.h"
#include "flecsi/partition/dcrs_utils.h"

const size_t output_rank(0);

TEST(dcrs, naive_partitioning) {
  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  auto naive = flecsi::dmp::naive_partitioning(sd);

  clog_set_output_rank(0);

  clog_container_one(info, "naive partitioning", naive, clog::space);
} // TEST

TEST(dcrs, simple2d_8x8) {

  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  auto dcrs = flecsi::dmp::make_dcrs(sd);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank == 0) {
    // Note: These assume that this test is run with 5 ranks.
    const std::vector<size_t> offsets =
      { 0, 2, 5, 8, 11, 14, 17, 20, 22, 25, 29, 33, 37 };

    const std::vector<size_t> indices =
      { 1, 8, 0, 2, 9, 1, 3, 10, 2, 4, 11, 3, 5, 12, 4, 6, 13, 5, 7, 14, 6,
        15, 0, 9, 16, 1, 8, 10, 17, 2, 9, 11, 18, 3, 10, 12, 19 };

    CINCH_ASSERT(EQ, dcrs.offsets, offsets);
    CINCH_ASSERT(EQ, dcrs.indices, indices);
  } // if

} // TEST

TEST(dcrs, simple2d_16x16) {

  flecsi::io::simple_definition_t sd("simple2d-16x16.msh");
  auto dcrs = flecsi::dmp::make_dcrs(sd);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#if 0
  if(rank == output_rank) {
    std::cout << dcrs << std::endl;
  } // if
#endif

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
