/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// system includes
#include <cinchtest.h>
#include <vector>

// user includes
#include "../../specializations/burton.h"

// put the flexi namespace up front
using namespace flexi;

//=============================================================================
//! \brief Fixture for testing the partitioner.
//=============================================================================
class PartitionTest : public ::testing::Test {

protected:

  //---------------------------------------------------------------------------
  // Types
  //---------------------------------------------------------------------------
  
  //! \brief the mesh type
  using mesh_t = burton_mesh_t;

  //! \brief the vertex type
  using vertex_t = mesh_t::vertex_t;

  //! \brief the cell type
  using cell_t = mesh_t::cell_t;


  //---------------------------------------------------------------------------
  //! \brief the test setup function
  //---------------------------------------------------------------------------
  virtual void SetUp() {

    // create the individual vertices
    std::vector<vertex_t*> vs;
    
    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
	auto v =
	  mesh_.create_vertex({double(i)+ 0.1*pow(double(j),1.8), 1.5*double(j)});
	v->setRank(1);
	vs.push_back(v);
      }

    }

    // define each cell
    size_t width1 = width + 1;

    for(size_t j = 0; j < height; ++j){
      for(size_t i = 0; i < width; ++i){
	auto c = 
	  mesh_.create_cell({vs[i + j * width1],
		vs[i + 1 + j * width1],
		vs[i + 1 + (j + 1) * width1],
		vs[i + (j + 1) * width1]});
      }
    }

    // now finalize the mesh setup
    mesh_.init();
  }

  //---------------------------------------------------------------------------
  //! \brief the test teardown function 
  //---------------------------------------------------------------------------
  virtual void TearDown() { }

  //---------------------------------------------------------------------------
  // Data members
  //---------------------------------------------------------------------------

  //! \brief the mesh object used for testing
  mesh_t mesh_;

  //! \brief number of vertices in width of domain
  const size_t width = 10;
  //! \brief number of vertices in height of domain
  const size_t height = 20;

};

//=============================================================================
//! \brief A simple partion test
//=============================================================================
TEST_F(PartitionTest, partition_simple) {

} // TEST_F


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
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
