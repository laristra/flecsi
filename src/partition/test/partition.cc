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
#include <iostream>
#include <metis.h>
#include <vector>

// user includes
#include "../../specializations/burton.h"

// put the flexi namespace up front
using namespace flexi;
using namespace std;

//=============================================================================
//! \brief Fixture for testing the partitioner.
//=============================================================================
class partition : public ::testing::Test {

protected:

  //---------------------------------------------------------------------------
  // Types
  //---------------------------------------------------------------------------
  
  //! \brief the mesh type
  using mesh_t = burton_mesh_t;

  //! \brief the real type
  using real_t = mesh_t::real_t;

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
TEST_F(partition, simple) {

  using std::endl;
  using std::vector;

  // get metis' index/real type ( metis has no namespace, so all
  // defines provided by metis.h are in the default namespace.
  using index_t = ::idx_t;
  using real_t  = ::real_t;


  // get the number of cells in the mesh
  index_t num_cells = mesh_.num_cells();
  index_t num_verts = mesh_.num_vertices();

  // now count the total number of vertices in each cell
  index_t tot_vert( 0 );
  for (auto c : mesh_.cells()) 
    tot_vert += mesh_.vertices(c).size();

  // create storage for element info
  vector<index_t> eptr( num_cells+1 );
  vector<index_t> eind( tot_vert );

  // now create the adjacency list
  index_t vert_cnt( 0 );
  eptr[0] = 0;
  for ( auto c : mesh_.cells() ) {
    for ( auto v : mesh_.vertices(c) ) {
      eind[ vert_cnt++ ] = v->id();
    }
    eptr[ c->id() + 1 ] = vert_cnt;
  }
  
  index_t ncommon = 2; // Specifies the number of common nodes that
                       // two elements must have in order to put an
                       // edge between them in the dual graph

  index_t nparts = 4;  // The number of parts to partition the mesh.

  index_t objval;
  vector<index_t> epart( num_cells );
  vector<index_t> npart( num_verts );

  CINCH_CAPTURE() << "Partitioning...";


#if 1  
  // subdivide based on the cells
  auto ret =  METIS_PartMeshDual( &num_cells, 
                                  &num_verts, 
                                  eptr.data(), 
                                  eind.data(), 
                                  nullptr, 
                                  nullptr,
                                  &ncommon, 
                                  &nparts, 
                                  nullptr, 
                                  nullptr, 
                                  &objval,
                                  epart.data(), 
                                  npart.data() );

#else
  // subdivide based on the nodes
  auto ret =  METIS_PartMeshNodal( &num_cells, 
                                   &num_verts, 
                                   eptr.data(), 
                                   eind.data(), 
                                   nullptr, 
                                   nullptr,
                                   &nparts, 
                                   nullptr, 
                                   nullptr, 
                                   &objval,
                                   epart.data(), 
                                   npart.data() );
#endif

  CINCH_CAPTURE() << "done." << endl;

  ASSERT_EQ( ret, METIS_OK );

  CINCH_CAPTURE() << "edgecuts = " << objval << endl;

  // check stuff on a cell basis
  vector<index_t> cells_per_part( nparts, 0 );

  for ( auto c : mesh_.cells() ) {
    CINCH_CAPTURE() << "----   cell: " << std::setw(4) << c->id() 
              << ", partition: "  << std::setw(2) << epart[ c->id() ] 
              << endl;
    for ( auto v : mesh_.vertices(c) ) {
      CINCH_CAPTURE() << "++++ vertex: " << std::setw(4) << v->id() 
                << ", partition: " << std::setw(2) << npart[ v->id() ] 
                << endl;
    }
    cells_per_part[ epart[ c->id() ] ] ++;
    CINCH_CAPTURE() << endl;
  }
  

  // check stuff on a node basis
  vector<index_t> verts_per_part( nparts, 0 );

  for ( auto v : mesh_.vertices() )
    verts_per_part[ npart[ v->id() ] ] ++;

  // print partition info
  for ( index_t i=0; i<nparts; i++ ) {
    CINCH_CAPTURE() << "partition: " << i+1 << endl;
    CINCH_CAPTURE() << "++++ number of cells: " << cells_per_part[i] << endl;
    CINCH_CAPTURE() << "++++ number of verts: " << verts_per_part[i] << endl;
    CINCH_CAPTURE() << endl;
  }
  

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("partition.blessed"));
    
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
