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
#include <scotch.h>
#include <vector>

// user includes
#include "../../specializations/burton.h"

// put the flexi namespace up front
using namespace flexi;
using namespace std;

// explicitly use some stuff
using std::endl;
using std::vector;

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
    vector<vertex_t*> vs;
    
    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
	auto v =
	  mesh_.create_vertex({double(i)+ 0.1*pow(double(j),1.8), 1.5*double(j)});
	v->set_rank(1);
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
  //! \brief run a final check on the results
  //!
  //! This function basically just dumps the output
  //!  
  //! \param[in] cell_part The new cell partitioning
  //! \param[in] vert_part The new vertex partitioning
  //---------------------------------------------------------------------------

  template < typename index_t >
  void check( const index_t * cell_part, const index_t * vert_part ) {

    // check stuff on a cell basis
    vector<index_t> cells_per_part( nparts_, 0 );

    for ( auto c : mesh_.cells() ) {
      CINCH_CAPTURE() << "----   cell: " << std::setw(4) << c->id() 
                      << ", partition: "  << std::setw(2) << cell_part[ c->id() ] 
                      << endl;
      for ( auto v : mesh_.vertices(c) ) {
        CINCH_CAPTURE() << "++++ vertex: " << std::setw(4) << v->id() 
                        << ", partition: " << std::setw(2) << vert_part[ v->id() ] 
                        << endl;
      }
      cells_per_part[ cell_part[ c->id() ] ] ++;
      CINCH_CAPTURE() << endl;
    }
    
    
    // check stuff on a node basis
    vector<index_t> verts_per_part( nparts_, 0 );
    
    for ( auto v : mesh_.vertices() )
      verts_per_part[ vert_part[ v->id() ] ] ++;
    
    // print partition info
    for ( index_t i=0; i<nparts_; i++ ) {
      CINCH_CAPTURE() << "partition: " << i+1 << endl;
      CINCH_CAPTURE() << "++++ number of cells: " << cells_per_part[i] << endl;
      CINCH_CAPTURE() << "++++ number of verts: " << verts_per_part[i] << endl;
      CINCH_CAPTURE() << endl;
    }
    
  }
    
  //---------------------------------------------------------------------------
  //! \brief Determine the cell-cell adjacency information
  //!  
  //! \param[out] cell_idx   The cell-cell adjacency starting index for each cell
  //! \param[out] cell_neigh The cell-cell adjacency information
  //---------------------------------------------------------------------------
  
  template < typename index_t >
  void create_cell_adjacency( vector<index_t> &cell_idx, vector<index_t> &cell_neigh ) {

    cell_idx[0] = 0;
    
#if 1
    
    for ( auto c : mesh_.cells() ) 
      for ( auto e : mesh_.edges(c) ) {
        // 2d specific, only add the non-c cell
        auto neigh =  mesh_.cells(e).toVec();
        if ( neigh[1] == c ) 
          cell_neigh.push_back( neigh[0]->id() );
        else if ( neigh[0] == c )
          cell_neigh.push_back( neigh[1]->id() );
        else
          FAIL();
      }
    
#else
    
    index_t neigh_cnt( 0 );

    for ( auto c : mesh_.cells() ) {
      for ( auto neigh : mesh_.cells(c) ) {
        cell_neigh.push_back( neigh->id() );
        neigh_cnt++;
      }
      cell_idx[ c->id() + 1 ] = neigh_cnt;
    }
    
#endif

  }
  
  //---------------------------------------------------------------------------
  //! \brief Partition the vertices based on the cell partitioning
  //!  
  //! \param[in]  cell_part  The cell partitioning
  //! \param[out] vert_part  The vertex partitionion
  //---------------------------------------------------------------------------
  template < typename index_t >
  void partition_vertices( const index_t * cell_part, index_t * vert_part ) {

    for ( auto v : mesh_.vertices() ) {
      auto cells = mesh_.cells(v);
      index_t owner_id( cells.begin()->id() );
      for ( auto c : cells ) 
        owner_id = std::min( (index_t)c->id(), owner_id );
      vert_part[ v->id() ] = cell_part[ owner_id ];
    }

  }

  
  //---------------------------------------------------------------------------
  // Data members
  //---------------------------------------------------------------------------

  //! \brief the mesh object used for testing
  mesh_t mesh_;

  //! \brief number of vertices in width of domain
  const size_t width = 10;
  //! \brief number of vertices in height of domain
  const size_t height = 20;

  //! \brief The number of parts to partition the mesh.
  const size_t nparts_ = 4;

};



//=============================================================================
//! \brief A simple partion test using metis mesh partitioning
//=============================================================================
TEST_F(partition, metis_mesh) {

  // get metis' index/real type ( metis has no namespace, so all
  // defines provided by metis.h are in the default namespace.
  using index_t = ::idx_t;

  //---------------------------------------------------------------------------
  // Extract the cell / vertex information
  //---------------------------------------------------------------------------

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

  //---------------------------------------------------------------------------
  // Partition with Metis
  //---------------------------------------------------------------------------

  // prepare metis input / output
  
  index_t nparts = nparts_;  // The number of parts to partition the mesh.

  index_t ncommon = 2; // Specifies the number of common nodes that
                       // two elements must have in order to put an
                       // edge between them in the dual graph

  index_t objval;
  vector<index_t> cell_part( num_cells );
  vector<index_t> vert_part( num_verts );

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
                                  cell_part.data(), 
                                  vert_part.data() );

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
                                   cell_part.data(), 
                                   vert_part.data() );
#endif

  ASSERT_EQ( ret, METIS_OK );

  CINCH_CAPTURE() << "done." << endl;


  //---------------------------------------------------------------------------
  // Final Checks
  //---------------------------------------------------------------------------

  check( cell_part.data(), vert_part.data() );


  ASSERT_TRUE(CINCH_EQUAL_BLESSED("metis_mesh.blessed"));
    
} // TEST_F



//=============================================================================
//! \brief A simple partion test using standard metis graph partitioning
//=============================================================================
TEST_F(partition, metis) {

  // get metis' index/real type ( metis has no namespace, so all
  // defines provided by metis.h are in the default namespace.
  using index_t = ::idx_t;

  //---------------------------------------------------------------------------
  // Extract the cell / vertex information
  //---------------------------------------------------------------------------

  // get the number of cells in the mesh
  index_t num_cells = mesh_.num_cells();
  index_t num_verts = mesh_.num_vertices();

  // create storage for element info
  vector<index_t> cell_idx( num_cells+1 );
  vector<index_t> cell_neigh;

  // now create the adjacency list
  create_cell_adjacency( cell_idx, cell_neigh );
  index_t neigh_cnt = cell_neigh.size();

  //---------------------------------------------------------------------------
  // Partition with Metis
  //---------------------------------------------------------------------------

  // prepare metis input / output
  
  index_t nparts = nparts_;  // The number of parts to partition the mesh.

  index_t ncon = 1; // The number of balancing constraints. It should
                    // be at least 1.

  index_t objval;
  vector<index_t> cell_part( num_cells );

  CINCH_CAPTURE() << "Partitioning...";


  // subdivide based on the cells
  auto ret =  METIS_PartGraphKway( &num_cells, 
                                   &ncon, 
                                   cell_idx.data(), 
                                   cell_neigh.data(), 
                                   nullptr, 
                                   nullptr,
                                   nullptr,
                                   &nparts, 
                                   nullptr, 
                                   nullptr, 
                                   nullptr,
                                   &objval,
                                   cell_part.data() );

  ASSERT_EQ( ret, METIS_OK );

  // decide on who owns the vertices
  vector<index_t> vert_part( num_verts );
  partition_vertices( cell_part.data(), vert_part.data() );

  CINCH_CAPTURE() << "done." << endl;


  //---------------------------------------------------------------------------
  // Final Checks
  //---------------------------------------------------------------------------

  check( cell_part.data(), vert_part.data() );


  ASSERT_TRUE(CINCH_EQUAL_BLESSED("metis.blessed"));
    
} // TEST_F




//=============================================================================
//! \brief A simple partion test using scotch
//=============================================================================
TEST_F(partition, scotch) {



  // get scotch' index/real type ( metis has no namespace, so all
  // defines provided by scotch.h are in the default namespace.
  using index_t = ::SCOTCH_Num;


  //---------------------------------------------------------------------------
  // create cell adjacency list
  //---------------------------------------------------------------------------

  // get the number of cells in the mesh
  index_t num_cells = mesh_.num_cells();
  index_t num_verts = mesh_.num_vertices();


  // create storage for element info
  vector<index_t> cell_idx( num_cells+1 );
  vector<index_t> cell_neigh;

  // now create the adjacency list
  create_cell_adjacency( cell_idx, cell_neigh );
  index_t neigh_cnt = cell_idx[ num_cells ];

  //---------------------------------------------------------------------------
  // Partition with Scotch
  //---------------------------------------------------------------------------

  index_t nparts = nparts_;  // The number of parts to partition the mesh.

  // build a scotch graph
  CINCH_CAPTURE() << "Partitioning...";

  SCOTCH_Graph  graph_data;
  SCOTCH_graphInit(&graph_data);

  auto ret =
    SCOTCH_graphBuild( &graph_data,
                       0,                  /* baseval; 0 to n -1 numbering */
                       num_cells,          /* vertnbr */
                       cell_idx.data(),    /* verttab */
                       NULL,               /* vendtab: verttab + 1 or NULL */
                       NULL,               /* velotab: vertex weights */
                       NULL,               /* vlbltab; vertex labels */
                       neigh_cnt,          /* edgenbr */
                       cell_neigh.data(),  /* edgetab */
                       NULL );             /* edlotab */
  
  ASSERT_EQ( ret, 0 );

  // check it
  ret = SCOTCH_graphCheck(&graph_data);
  ASSERT_EQ( ret, 0 );
  
  // now partition

  vector<index_t> cell_part( num_cells );

  SCOTCH_Strat  strat_data;
  SCOTCH_stratInit(&strat_data);

  ret = SCOTCH_graphPart(&graph_data, nparts, &strat_data, cell_part.data());
  ASSERT_EQ( ret, 0 );

  // clean up
  SCOTCH_stratExit(&strat_data);
  SCOTCH_graphExit(&graph_data);


  // decide on who owns the vertices
  vector<index_t> vert_part( num_verts );
  partition_vertices( cell_part.data(), vert_part.data() );

  CINCH_CAPTURE() << "done." << endl;


  //---------------------------------------------------------------------------
  // Final Checks
  //---------------------------------------------------------------------------

  check( cell_part.data(), vert_part.data() );
  

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("scotch.blessed"));
  
    
} // TEST_F


#if 0

//=============================================================================
//! \brief A simple partion test using morton ordering
//=============================================================================
TEST_F(partition, morton) {


  // use large integers
  using index_t = uint64_t;


  //---------------------------------------------------------------------------
  // get the bounding box
  //---------------------------------------------------------------------------
  point_t min_pt, max_pt;
  bounding_box( min_pt, max_pt );

  morton( xyz, bbox, depth, id )

  // decide on who owns the vertices
  vector<index_t> vert_part( num_verts );
  partition_vertices( cell_part.data(), vert_part.data() );

  CINCH_CAPTURE() << "done." << endl;


  //---------------------------------------------------------------------------
  // Final Checks
  //---------------------------------------------------------------------------

  check( cell_part.data(), vert_part.data() );
  

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("morton.blessed"));
  
    
} // TEST_F

#endif

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
