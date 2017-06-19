/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_io_exodus_definition_h
#define flecsi_io_exodus_definition_h

/// \file
/// \date Initial file creation: Nov 21, 2016

// user includes
#include "flecsi/topology/mesh_definition.h"
#include "flecsi/utils/logging.h"
#include "flecsi/utils/compressed_storage.h"

// thirdparty includes
#include <exodusII.h>

// system includes
#include <fstream>
#include <unordered_map>
#include <string>

namespace flecsi {
namespace io {

////////////////////////////////////////////////////////////////////////////////
/// \brief This is the three-dimensional mesh reader and writer based on the 
///        Exodus format.
///
/// io_base_t provides registrations of the exodus file extensions.
////////////////////////////////////////////////////////////////////////////////
template< int D, typename T >
class exodus_definition__ : public topology::mesh_definition__<D>
{

public:

  //============================================================================
  // Typedefs
  //============================================================================

  //! the instantiated mesh definition type
  using mesh_definition_t = topology::mesh_definition__<D>;

  //! the size type
  using size_t = std::size_t;
  //! \brief the counter type
  using counter_t = flecsi::utils::counter_t;
  //! \brief the floating point type
  using real_t = T;
  //! \brief the type used for indexing arrays
  using index_t = std::size_t;

  //! the point type
  using typename mesh_definition_t::point_t;

  using mesh_definition_t::dimension;

  //! \brief an alias for the vector class
  template< typename U >
  using vector = typename std::vector<U>;

  //! an alias for the matrix class
  template <typename U>
  using sparse_matrix = typename utils::compressed_row_storage<U>;

  //============================================================================
  // Constructors
  //============================================================================
  
  //! \brief Default constructor
  exodus_definition__() = default;

  //! \brief Constructor with filename
  //! \param [in] filename  The name of the file to load
  exodus_definition__(const std::string & filename )
  {
    read(filename);
  }
  
  /// Copy constructor (disabled)
  exodus_definition__(const exodus_definition__ &) = delete;

  /// Assignment operator (disabled)
  exodus_definition__ & operator = (const exodus_definition__ &) = delete;

  /// Destructor
  ~exodus_definition__() = default;

  //============================================================================
  //! \brief open the file for reading or writing
  //! \param [in] name  The name of the file to open.
  //! \return The exodus handle for the open file.
  //============================================================================
  static auto open( const std::string &name ) 
  {
   

#ifdef DEBUG
      // useful for debug
      ex_opts (EX_ABORT | EX_VERBOSE);
#endif

    // size of floating point variables used in app.
    int app_word_size = sizeof(real_t);

    // size of floating point stored in name.
    int exo_word_size = 0;
    // the version number
    float version;
    
    // open the file
    auto exo_id = ex_open(
      name.c_str(), EX_READ, &app_word_size, &exo_word_size, &version);
    if ( exo_id < 0 ) 
      clog_fatal( 
        "Problem opening exodus file, ex_open() returned " << exo_id 
      );
    
    return exo_id;
  }

  //============================================================================
  //! \brief close the file once completed reading or writing
  //! \param [in] exo_id  The exodus file id.
  //============================================================================
  static void close( int exo_id ) 
  {
    auto status = ex_close(exo_id);
    if ( status ) 
      clog_fatal( 
        "Problem closing exodus file, ex_close() returned " << exo_id 
      );
  }
    
  //============================================================================
  //! \brief check the integer status
  //! \param [in] exo_id  The exodus file id.
  //! \return true if exodus is using 64 bit integers
  //============================================================================
  static auto is_int64( int exo_id ) 
  {
    return  ( ex_int64_status(exo_id) & EX_IDS_INT64_API );
  }


  //============================================================================
  //! \brief read the exodus parameters from a file.
  //! \param [in] exo_id  The exodus file id.
  //! \return the exodus parameters
  //============================================================================
  static auto read_params( int exo_id ) 
  { 
    ex_init_params exo_params;

    // get the initialization parameters
    auto status = ex_get_init_ext(exo_id, &exo_params);
    if ( status )
      clog_fatal( 
        "Problem getting exodus file parameters, ex_get_init_ext() returned " 
        << status 
      );

    return exo_params;
  }


  //============================================================================
  //! \brief read the coordinates of the mesh from a file.
  //! \param [in] exo_id  The exodus file id.
  //! \return the vertex coordinates
  //============================================================================
  static auto read_point_coords( int exo_id ) 
  { 

    // get the exodus parameters
    auto exo_params = read_params( exo_id );
    
    // get the number of nodes
    auto num_nodes = exo_params.num_nodes;
    if ( num_nodes <= 0 )  
      clog_fatal(
        "Exodus file has zero nodes, or parmeters haven't been read yet."
      );

    // the number of dimensions
    auto num_dims = exo_params.num_dim;

    // read nodes
    vector<real_t> vertex_coord( num_dims * num_nodes );

    // exodus is kind enough to fetch the data in the real type we ask for
    auto status = ex_get_coord( 
      exo_id, 
      vertex_coord.data(), 
      vertex_coord.data()+num_nodes, 
      vertex_coord.data()+2*num_nodes
    );

    if (status)
      clog_fatal(
        "Problem getting vertex coordinates from exodus file, " <<
        " ex_get_coord() returned " << status 
      );

    return vertex_coord;

  }
  
  //============================================================================
  //! \brief read the face blocks from an exodus file.
  //! \param [in] exo_id  The exodus file id.
  //! \return the status of the file
  //============================================================================
  template< typename U >
  static auto read_face_blocks( int exoid )
  {
    // some type aliases
    using ex_index_t = U;

    // storage for element verts
    sparse_matrix<index_t> faces;
    
    // figure out how many element blocks there are
    auto exo_params = read_params(exoid);
    auto num_face_blk = exo_params.num_face_blk;
    if ( num_face_blk == 0 ) return faces;
    
    // get the face block ids 
    vector<ex_index_t> face_block_ids( num_face_blk );
    auto status = ex_get_ids(exoid, EX_FACE_BLOCK, face_block_ids.data() );
    if ( status )
      clog_fatal( 
        "Problem reading face block ids, ex_get_ids() returned " << status 
      );

    // read each block
    for ( int iblk=0; iblk<num_face_blk; iblk++ ) {

      auto face_blk_id = face_block_ids[iblk];

      // get the info about this block
      ex_index_t num_face_this_blk = 0;
      ex_index_t num_faces_per_face = 0;
      ex_index_t num_edges_per_face = 0;
      ex_index_t num_nodes_per_face = 0;
      ex_index_t num_attr = 0;
      char face_type[MAX_STR_LENGTH];

      status = ex_get_block(
        exoid, EX_FACE_BLOCK, face_blk_id, face_type, &num_face_this_blk, 
        &num_nodes_per_face, &num_edges_per_face, &num_faces_per_face, &num_attr );
      if ( status )
        clog_fatal( 
          "Problem reading face block, ex_get_block() returned " << status 
        );
      

      // the number of nodes per face is really the number of
      // nodes in the whole block ( includes duplicate / overlapping
      // nodes )
      auto num_nodes_this_blk = num_nodes_per_face;

      // get the number of nodes per face
      vector<int> face_node_counts( num_nodes_this_blk );
      status = ex_get_entity_count_per_polyhedra( 
        exoid, EX_FACE_BLOCK, face_blk_id, face_node_counts.data() );
      if ( status )
        clog_fatal( 
          "Problem reading face node info, " <<
          "ex_get_entity_count_per_polyhedra() returned " << status 
        );
      
      // read face definitions
      vector<ex_index_t> face_nodes( num_nodes_this_blk );
      status = ex_get_conn( 
        exoid, EX_FACE_BLOCK, face_blk_id, face_nodes.data(), nullptr, nullptr );
      if (status)
        clog_fatal(
          "Problem getting face connectivity, ex_get_conn() " <<
          "returned " << status
        );
        
      // storage for element verts
      vector<index_t> face_vs;
      face_vs.reserve( face_node_counts[0] );
        
      // create faces in mesh
      for (counter_t e=0, base=0; e < num_face_this_blk; ++e) {
        face_vs.clear();
        // get the number of nodes
        num_nodes_per_face = face_node_counts[e];
        // copy local vertices into vector ( exodus uses 1 indexed arrays )
        for ( auto v=0;  v<num_nodes_per_face; v++ )
          face_vs.emplace_back( face_nodes[base+v] - 1 );
        // add face to master list
        faces.push_back( face_vs.begin(), face_vs.end() );
        // base offset into face conn
        base += num_nodes_per_face;
      }

    }
  
    return faces;
  }


  //============================================================================
  //! \brief read the element blocks from an exodus file.
  //! \param [in] exo_id  The exodus file id.
  //! \return the status of the file
  //============================================================================
  template< typename U >
  static auto read_element_blocks( int exoid )
  {
    // some type aliases
    using ex_index_t = U;
    
    // first load any face blocks if there are any
    auto faces = read_face_blocks<U>( exoid );

    // figure out how many element blocks there are
    auto exo_params = read_params(exoid);
    auto num_elem_blk = exo_params.num_elem_blk;
    auto num_dims = exo_params.num_dim;

    // get the block ids
    vector<ex_index_t> blockids( num_elem_blk );
    auto status = ex_get_elem_blk_ids(exoid, blockids.data() );
    if (status)
      clog_fatal( 
        "Problem getting block ids, ex_get_elem_blk_ids() returned " << status
      );
        
    // storage for element verts
    sparse_matrix<index_t> elements;

    // read each block
    for ( int iblk=0; iblk<num_elem_blk; iblk++ ) {

      auto elem_blk_id = blockids[iblk];

      char block_name[MAX_LINE_LENGTH];
      status = ex_get_name(exoid, EX_ELEM_BLOCK, elem_blk_id, block_name);
      if ( status )
        clog_fatal(
          "Problem reading block name, ex_get_name returned " << status 
        );
      
      // get the info about this block
      ex_index_t num_elem_this_blk = 0;
      ex_index_t num_faces_per_elem = 0;
      ex_index_t num_edges_per_elem = 0;
      ex_index_t num_nodes_per_elem = 0;
      ex_index_t num_attr = 0;
      char elem_type[MAX_STR_LENGTH];
      status = ex_get_block(
        exoid, 
        EX_ELEM_BLOCK, 
        elem_blk_id, 
        elem_type, 
        &num_elem_this_blk, 
        &num_nodes_per_elem, 
        &num_edges_per_elem, 
        &num_faces_per_elem, 
        &num_attr 
      );
      if ( status )
        clog_fatal( 
          "Problem reading element block, ex_get_block() returned " << status 
        );

      //------------------------------------------------------------------------
      // polygon data
      if ( strcmp("nsided",elem_type) == 0 || 
           strcmp("NSIDED",elem_type) == 0 ) 
      {

        // the number of nodes per element is really the number of nodes 
        // in the whole block
        auto num_nodes_this_blk = num_nodes_per_elem;


        // get the number of nodes per element
        vector<int> elem_node_counts(num_elem_this_blk);
        status = ex_get_entity_count_per_polyhedra( 
          exoid, EX_ELEM_BLOCK, elem_blk_id, elem_node_counts.data() );
        if (status)
          clog_fatal( 
            "Problem getting element node numbers, " <<
            "ex_get_entity_count_per_polyhedra() returned " << status
          );

        // read element definitions
        vector<ex_index_t> elem_nodes(num_nodes_this_blk);
        status = ex_get_elem_conn(exoid, elem_blk_id, elem_nodes.data());
        if (status)
          clog_fatal(
            "Problem getting element connectivity, ex_get_elem_conn() " <<
            "returned " << status
          );
        
        // storage for element verts
        vector<index_t> elem_vs;
        elem_vs.reserve( num_dims * elem_node_counts[0] );
        
        // create cells in mesh
        size_t base = 0;
        for (counter_t e = 0; e < num_elem_this_blk; ++e) {
          elem_vs.clear();
          // get the number of nodes
          num_nodes_per_elem = elem_node_counts[e];
          // copy local vertices into vector ( exodus uses 1 indexed arrays )
          for ( int v=0;  v<num_nodes_per_elem; v++ )
            elem_vs.emplace_back( elem_nodes[base+v] - 1 );
          // add the row
          elements.push_back( elem_vs.begin(), elem_vs.end() );
          // base offset into elt_conn
          base += num_nodes_per_elem;
        }

      }
      //------------------------------------------------------------------------
      // polygon data
      else if ( strcmp("nfaced",elem_type) == 0 || 
                strcmp("NFACED",elem_type) == 0 ) 
      {

        // the number of faces per element is really the number of
        // faces in the whole block ( includes duplicate / overlapping
        // nodes )
        auto num_face_this_blk = num_faces_per_elem;

        // get the number of nodes per element
        vector<int> elem_face_counts( num_face_this_blk );
        status = ex_get_entity_count_per_polyhedra( 
          exoid, EX_ELEM_BLOCK, elem_blk_id, elem_face_counts.data() );
        if ( status )
          clog_fatal( 
            "Problem reading element node info, " <<
            "ex_get_entity_count_per_polyhedra() returned " << status 
          );

        // read element definitions
        vector<ex_index_t> elem_faces( num_face_this_blk );
        status = ex_get_conn( 
          exoid, 
          EX_ELEM_BLOCK, 
          elem_blk_id, 
          nullptr, 
          nullptr, 
          elem_faces.data() 
        );
        if (status)
          clog_fatal(
            "Problem getting element connectivity, ex_get_conn() " <<
            "returned " << status
          );
        
        // storage for element faces
        vector<index_t> elem_vs;
        elem_vs.reserve( elem_face_counts[0] );
        
        // create cells in mesh
        for (counter_t e=0, base=0; e < num_elem_this_blk; ++e) {
          // reset storage
          elem_vs.clear();
          // get the number of faces
          num_faces_per_elem = elem_face_counts[e];
          // copy local vertices into vector ( exodus uses 1 indexed arrays )
          for ( int v=0;  v<num_faces_per_elem; v++ ) {
            auto id = elem_faces[base+v] - 1;            
            elem_vs.insert( 
              elem_vs.end(), 
              faces.row_begin(id), 
              faces.row_end(id)
            );
          }
          // base offset into elt_conn
          base += num_faces_per_elem;
          // remove duplicates
          std::sort( elem_vs.begin(), elem_vs.end() );
          auto last = std::unique( elem_vs.begin(), elem_vs.end() );
          // add vertex list to master list
          elements.push_back( elem_vs.begin(), last );
        }

      }
      //------------------------------------------------------------------------
      // fixed element size
      else {

        // read element definitions
        vector<ex_index_t> elt_conn(num_elem_this_blk * num_nodes_per_elem);
        status = ex_get_elem_conn(exoid, elem_blk_id, elt_conn.data());
        if (status)
          clog_fatal(
            "Problem getting element connectivity, ex_get_elem_conn() " <<
            "returned " << status
          );
        
        // storage for element verts
        vector<index_t> elem_vs;
        elem_vs.reserve( num_nodes_per_elem );
        
        // create cells in mesh
        for (counter_t e = 0; e < num_elem_this_blk; ++e) {
          elem_vs.clear();
          // base offset into elt_conn
          auto b = e*num_nodes_per_elem;
          // copy local vertices into vector ( exodus uses 1 indexed arrays )
          for ( int v=0;  v<num_nodes_per_elem; v++ )
            elem_vs.emplace_back( elt_conn[b+v]-1 );
          // add the row
          elements.push_back( elem_vs.begin(), elem_vs.end() );
        }

      } // element type
      //------------------------------------------------------------------------

    }
    // end blocks
    
    return elements;

  }


  //============================================================================
  //! \brief Implementation of exodus mesh read for burton specialization.
  //!
  //! \param[in] name Read burton mesh \e m from \e name.
  //! \param[out] m Populate burton mesh \e m with contents of \e name.
  //!
  //! \return Exodus error code. 0 on success.
  //============================================================================
  void read( const std::string &name )
  {

    clog(info) << "Reading mesh from: " << name << std::endl;

    // open the exodus file
    auto exoid = open( name );
    if ( exoid < 0 )
      clog_fatal( "Problem reading exodus file" );

    // get the initialization parameters
    auto exo_params = read_params(exoid);

    // verify mesh dimension
    if ( dimension() != exo_params.num_dim )
      clog_fatal(
        "Exodus dimension mismatch: Expected dimension (" 
        << dimension() << ") /= Exodus dimension (" << exo_params.num_dim 
        << ")" 
      );

    // check the integer type used in the exodus file
    auto int64 = is_int64(exoid);

    // read coordinates
    vertices_ = read_point_coords(exoid);
    clog_assert( 
      vertices_.size() == dimension()*exo_params.num_nodes,
      "Mismatch in read vertices"
    );
    
    // read blocks
    if ( int64 )
      cells_ = read_element_blocks<long long>( exoid );
    else 
      cells_ = read_element_blocks<int>( exoid );
    
    clog_assert( 
      cells_.rows() == exo_params.num_elem,
      "Mismatch in read blocks"
    );

    // close the file
    close( exoid );
  }

  //============================================================================
  // Required Overrides
  //============================================================================

  /// Return the number of entities of a particular dimension
  /// \param [in] dim  The entity dimension to query.
  size_t num_entities( size_t dim ) const override
  {
    switch (dim)
    {
      case 0: 
        return vertices_.size()/dimension();
      case dimension(): 
        return cells_.rows();
      default:
        clog_fatal( 
          "Dimension out of range: 0 < " << dim << " </ " << dimension()
        );
        return 0;
    }
  }

  /// Return the set of vertices of a particular entity.
  /// \param [in] dimension  The entity dimension to query.
  /// \param [in] entity_id  The id of the entity in question.
  std::vector<size_t>
  vertices( 
    size_t dim,
    size_t entity_id
  )
  const override
  {
    switch (dim)
    {
      case 0:
        return {entity_id};
      case dimension(): 
        return cells_.row(entity_id);
      default:
        clog_fatal(
          "Dimension out of range: 0 < " << dim << " </ " << dimension() 
        );
        return {};
    }
  } // vertices

  /// Return the vertex coordinates for a certain id.
  /// \param [in] vertex_id  The id of the vertex to query.
  point_t vertex( size_t vertex_id ) const override
  {
    auto num_vertices = vertices_.size()/dimension();
    point_t p;
    for ( int i=0; i<dimension(); ++i )
      p[i] = vertices_[ i*num_vertices + vertex_id ];
    return p;
  } // vertex

private:

  //============================================================================
  // Private data
  //============================================================================

  //! brief storage for element verts
  sparse_matrix<index_t> cells_;
  
  //! brief storage for vertex coordinates
  vector<real_t> vertices_;


};

} // namespace io
} // namespace flecsi

#endif // flecsi_io_exodus_definition_h

