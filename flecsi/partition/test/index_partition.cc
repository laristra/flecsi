/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include "flecsi/partition/index_partition.h"

#include <cinchtest.h>

#include <cmath>
#include <mpi.h>
#include <cereal/archives/binary.hpp>
#include <sstream>

using index_partition_t = flecsi::dmp::index_partition__<size_t>;

static constexpr size_t N = 8;

// test fixture
class ip_fixture_t : public ::testing::Test
{
protected:

  virtual
  void
  SetUp() override
  {
    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    size_t part = N/size;
    size_t rem = N%size;

    size_t start = rank*(part + (rem > 0 ? 1 : 0));
    size_t end = rank < rem ? start + part+1 : start + part;

    for(size_t j(0); j<N; ++j) {
      for(size_t i(start); i<end; ++i) {
        const size_t id = j*N+i;

        // exclusive
        if(i>start && i<end-1) {
          ip_.exclusive.push_back(id);
        }
        else if(rank == 0 && i==start) {
          ip_.exclusive.push_back(id);
        }
        else if(rank == size-1 && i==end-1) {
          ip_.exclusive.push_back(id);
        }
        else if(i==start) {
          ip_.shared.push_back(id);

          const size_t ghost_id = j*N+i-1;
          ip_.shared.push_back(ghost_id);
        }
        else if(i==end-1) {
          ip_.shared.push_back(id);

          const size_t ghost_id = j*N+i+1;
          ip_.shared.push_back(ghost_id);
        } // if
      } // for
    } // for
  } // SetUp

  virtual void TearDown() override {}

  index_partition_t ip_;

}; // ip_fixture_t

TEST_F(ip_fixture_t, basic) {
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank == 0) {
    for(auto i: ip_.exclusive) {
      CINCH_CAPTURE() << i << std::endl;
    } // for

    for(auto i: ip_.shared) {
      CINCH_CAPTURE() << i << std::endl;
    } // for

    for(auto i: ip_.ghost) {
      CINCH_CAPTURE() << i << std::endl;
    } // for
  } // if

} // basic

TEST_F(ip_fixture_t, cereal) {
  std::stringstream ss;

  // serialize the index partition
  {
  cereal::BinaryOutputArchive oarchive(ss);
  oarchive(ip_);
  } // scope

  // deserialize the index partition
  index_partition_t ip_in;
  {
  cereal::BinaryInputArchive iarchive(ss);
  iarchive(ip_in);
  } // scope

  CINCH_ASSERT(EQ, ip_, ip_in);
} // cereal

TEST(index_partition, compare_output) {
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank == 0) {
    CINCH_ASSERT(TRUE, CINCH_EQUAL_BLESSED("index_partition.blessed"));
  } // if
} // compare_output

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
