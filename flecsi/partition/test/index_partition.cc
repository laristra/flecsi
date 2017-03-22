/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include "flecsi/partition/index_partition.h"

#include <cinchtest.h>

#include <cmath>
#include <cassert>
#include <mpi.h>
#include <sstream>

using index_partition_t = flecsi::dmp::index_partition__<size_t>;
using ghost_info_t = index_partition_t::ghost_info_t;
using shared_info_t = index_partition_t::shared_info_t;

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

    std::vector<size_t> ntart_per_rank;
    for (int i =0; i<size; i++)
    {
      size_t start = i*(part + (rem > 0 ? 1 : 0));   
      ntart_per_rank.push_back(start);
    }
    
    size_t start = rank*(part + (rem > 0 ? 1 : 0));
    size_t end = rank < rem ? start + part+1 : start + part;

    std::vector<size_t> start_global_id;
	  size_t global_end=0;
	  for (int i=0; i<size; i++)
	  {
    	size_t start_i = rank*(part + (rem > 0 ? 1 : 0));
    	size_t end_i = rank < rem ? start_i + part+1 : start + part;
    	start_global_id.push_back(global_end);
    	global_end +=N*(end_i-start_i);
  	}//end for

    for(size_t j=0; j<N; ++j) {
    for(size_t i(start); i<end; ++i) {
      ghost_info_t ghost;
      const size_t id = j*N+i;
      // exclusive
      if(i>start && i<end-1) {
        ip_.exclusive.push_back(id);
        //std::cout << "rank: " << rank << " exclusive: " << id << std::endl;
      }
      else if(rank == 0 && i==start) {
        ip_.exclusive.push_back(id);
        //std::cout << "rank: " << rank << " exclusive: " << id << std::endl;
       }
      else if(rank == size-1 && i==end-1) {
        ip_.exclusive.push_back(id);
        //std::cout << "rank: " << rank << " exclusive: " << id << std::endl;
      }
      else if(i==start) {
          shared_info_t shared;
          shared.mesh_id = id;
          shared.global_id =0;
          shared.dependent_ranks.push_back(rank - 1);
          ip_.shared.push_back(shared);

          const size_t ghost_id = j*N+i-1;
          ghost.mesh_id =ghost_id ;
          ghost.global_id = start_global_id[rank-1]+(end-start)*j+1;
          ghost.rank =rank -1;
          ip_.ghost.push_back(ghost);
      }
      else if(i==end-1) {
          shared_info_t shared;
          shared.mesh_id = id;
          shared.global_id = 0;
          shared.dependent_ranks.push_back(rank + 1);
          ip_.shared.push_back(shared);

          const size_t ghost_id = j*N+i+1;
          ghost.mesh_id =ghost_id ;
          ghost.rank = rank+1 ;
          ghost.global_id = start_global_id[rank+1]+(end-start)*j;
          ip_.ghost.push_back(ghost);
      } // if
    } // for
  } // for

	//creating primary partitioning and filling global_id's for shared elements:
  int start_indx=0;
  size_t previous_indx=0;
  for (int i=0; i<ip_.exclusive.size(); i++){
    for (int j=start_indx; j<ip_.shared.size(); j++){
        if (ip_.exclusive[i]<ip_.shared_id(j))
        {
          ip_.primary.push_back(ip_.exclusive[i]);
          previous_indx=ip_.exclusive[i];
          j=ip_.shared.size()+1;
          start_indx=ip_.primary.size()-i-1;
        }//end if
        else
        {
          ip_.primary.push_back(ip_.shared_id(j));
          previous_indx=ip_.shared_id(j);
          ip_.shared[j].global_id = start_global_id[rank]+ip_.primary.size()-1;
          start_indx++;
        }//end else
      }//end for

      if (start_indx>(ip_.shared.size()-1))
      {
        if (ip_.exclusive[i]>previous_indx)
          ip_.primary.push_back(ip_.exclusive[i]);
      }

  }//end_for
  for (int i = start_indx; i< ip_.shared.size(); i++)
  {
      ip_.primary.push_back(ip_.shared_id(i));
      ip_.shared[i].global_id = start_global_id[rank]+ip_.primary.size()-1;
  }//end for


  if (size>1)
    assert (ip_.primary.size() == (ip_.exclusive.size()+ip_.shared.size()));

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

    for(size_t i=0; i< ip_.shared.size(); i++)
    {
      auto element_id=ip_.shared_id(i);
      CINCH_CAPTURE() << element_id << std::endl;
    } // for

    for(size_t i=0; i< ip_.ghost.size(); i++) {
      auto element_id=ip_.ghost_id(i);
      CINCH_CAPTURE() << element_id << std::endl;
    } // for
  } // if

} // basic

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
