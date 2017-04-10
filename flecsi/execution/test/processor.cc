/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/execution/default_driver.h"
#include "flecsi/execution/common/processor.h"

using namespace flecsi::execution;
using namespace flecsi::utils;

TEST(processor, sanity) {

  // Test the Debruijn sequence
  for(size_t i(0); i<32; ++i) {
    int mask = 1 << i;
    ASSERT_EQ(debruijn32_t::index(mask), i);
  } // for

  // Test loc interface
  ASSERT_TRUE(processor_loc({loc}));
  ASSERT_TRUE(processor_loc({loc | toc}));
  ASSERT_TRUE(processor_loc({loc | toc | mpi}));

  // Test toc interface
  ASSERT_TRUE(processor_toc({toc}));
  ASSERT_TRUE(processor_toc({loc | toc}));
  ASSERT_TRUE(processor_toc({loc | toc | mpi}));

  // Test mpi interface
  ASSERT_TRUE(processor_mpi({mpi}));
  ASSERT_TRUE(processor_mpi({loc | mpi}));
  ASSERT_TRUE(processor_mpi({loc | toc | mpi}));

  // Test mask to type logic
  ASSERT_EQ(mask_to_type({loc}), processor_type_t::loc);
  ASSERT_EQ(mask_to_type({toc}), processor_type_t::toc);
  ASSERT_EQ(mask_to_type({mpi}), processor_type_t::mpi);

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
