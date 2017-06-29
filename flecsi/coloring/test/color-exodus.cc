
/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchdevel.h>

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <fstream>
#include <mpi.h>

#include "flecsi/io/exodus_definition.h"
#include "flecsi/coloring/dcrs_utils.h"
#include "flecsi/coloring/parmetis_colorer.h"
#include "flecsi/coloring/mpi_communicator.h"
#include "flecsi/topology/closure_utils.h"
#include "flecsi/utils/set_utils.h"

clog_register_tag(coloring);


// some type aliases
using exodus_definition_2d_t =
  flecsi::io::exodus_definition__<2, double>;

using exodus_definition_3d_t =
  flecsi::io::exodus_definition__<3, double>;

using std::vector;


template< int THRU_DIM, typename MD >
void color_cells( const MD & md, const std::string & output_prefix ) 
{

  constexpr auto cell_dim = MD::dimension();

  // Set the output rank
  clog_set_output_rank(1);

  using entry_info_t = flecsi::coloring::entity_info_t;

  int comm_size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Create a communicator instance to get neighbor information.
  auto communicator = std::make_shared<flecsi::coloring::mpi_communicator_t>();
  
  //----------------------------------------------------------------------------
  // Cell Coloring
  //----------------------------------------------------------------------------
    
  // Create the dCRS representation for the distributed colorer.
  // This essentialy makes the graph of the dual mesh.
  auto dcrs = flecsi::coloring::make_dcrs(md);

  // Create a colorer instance to generate the primary coloring.
  auto colorer = std::make_shared<flecsi::coloring::parmetis_colorer_t>();

  // Create the primary coloring.
  auto primary = colorer->color(dcrs);

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "primary coloring", primary, clog::space);
  } // guard

  //----------------------------------------------------------------------------
  // Cell Closure.  However many layers of ghost cells are needed are found 
  // here.
  //----------------------------------------------------------------------------
    
  // Compute the dependency closure of the primary cell coloring
  // through vertex intersections (specified by last argument "1").
  // To specify edge or face intersections, use 2 (edges) or 3 (faces).
  auto closure = 
    flecsi::topology::entity_closure<cell_dim, cell_dim, THRU_DIM>(
      md, primary
    );

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "closure", closure, clog::space);
  } // guard

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nearest_neighbors = flecsi::utils::set_difference(closure, primary);

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "nearest neighbors", nearest_neighbors, clog::space);
  } // guard

  //----------------------------------------------------------------------------
  // Find one more layer of ghost cells now, these are needed to get some corner
  // cases right.
  //----------------------------------------------------------------------------

  // We can iteratively add halos of nearest neighbors, e.g.,
  // here we add the next nearest neighbors. For most mesh types
  // we actually need information about the ownership of these indices
  // so that we can deterministically assign rank ownership to vertices.
  auto nearest_neighbor_closure =
    flecsi::topology::entity_closure<cell_dim, cell_dim, THRU_DIM>(
      md, nearest_neighbors
    );

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "nearest neighbor closure",
    nearest_neighbor_closure, clog::space);
  } // guard

  // Subtracting out the closure leaves just the
  // next nearest neighbors.
  auto next_nearest_neighbors =
    flecsi::utils::set_difference(nearest_neighbor_closure, closure);

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "next nearest neighbor", next_nearest_neighbors,
    clog::space);
  } // guard

  // The union of the nearest and next-nearest neighbors gives us all
  // of the cells that might reference a vertex that we need.
  auto all_neighbors = flecsi::utils::set_union(nearest_neighbors,
    next_nearest_neighbors);

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "all neighbors", all_neighbors, clog::space);
  } // guard

  //----------------------------------------------------------------------------
  // Find exclusive, shared, and ghost cells..
  //----------------------------------------------------------------------------

  // Get the rank and offset information for our nearest neighbor
  // dependencies. This also gives information about the ranks
  // that access our shared cells.
  auto cell_nn_info =
    communicator->get_primary_info(primary, nearest_neighbors);

  // Create a map version of the local info for lookups below.
  std::unordered_map<size_t, size_t> primary_indices_map;
  {
  size_t offset(0);
  for(auto i: primary) {
    primary_indices_map[offset++] = i;
  } // for
  } // scope

  std::set<entry_info_t> exclusive_cells;
  std::set<entry_info_t> shared_cells;
  std::set<entry_info_t> ghost_cells;

  // Populate exclusive and shared cell information.
  {
  size_t offset(0);
  for(auto i: std::get<0>(cell_nn_info)) {
    if(i.size()) {
      shared_cells.insert(entry_info_t(primary_indices_map[offset],
        rank, offset, i));
    }
    else {
      exclusive_cells.insert(entry_info_t(primary_indices_map[offset],
        rank, offset, i));
    } // if
    ++offset;
  } // for
  } // scope
  
  // Populate ghost cell information.
  {
  size_t offset(0);
  for(auto i: std::get<1>(cell_nn_info)) {
    ghost_cells.insert(i);
  } // for
  } // scope

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "exclusive cells ", exclusive_cells, clog::newline);
  clog_container_one(info, "shared cells ", shared_cells, clog::newline);
  clog_container_one(info, "ghost cells ", ghost_cells, clog::newline);
  } // guard
  
  //----------------------------------------------------------------------------
  // Create some maps for easy lookups when determining the other dependent
  // closures.
  //----------------------------------------------------------------------------

  // Create a map version for lookups below.
  std::unordered_map<size_t, entry_info_t> shared_cells_map;
  {
  for(auto i: shared_cells) {
    shared_cells_map[i.id] = i;
  } // for
  } // scope

  // Get the rank and offset information for all relevant neighbor
  // dependencies. This information will be necessary for determining
  // shared vertices.
  auto cell_all_info = communicator->get_primary_info(primary, all_neighbors);

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, entry_info_t> remote_info_map;
  for(auto i: std::get<1>(cell_all_info)) {
    remote_info_map[i.id] = i;
  } // for

  // Get the intersection of our nearest neighbors with the nearest
  // neighbors of other ranks. This map of sets will only be populated
  // with intersections that are non-empty
  auto closure_intersection_map =
    communicator->get_intersection_info(nearest_neighbors);
  
  //----------------------------------------------------------------------------
  // Vertex Closure
  //----------------------------------------------------------------------------
    
  // Form the vertex closure
  auto vertex_closure = flecsi::topology::vertex_closure<cell_dim>(md, closure);

  // Assign vertex ownership
  vector<std::set<size_t>> vertex_requests(comm_size);
  std::set<entry_info_t> vertex_info;

  size_t offset(0);
  for(auto i: vertex_closure) {

    // Get the set of cells that reference this vertex.
    auto referencers = flecsi::topology::vertex_referencers<cell_dim>(md, i);

    {
    clog_tag_guard(coloring);
    clog_container_one(info, i << " referencers", referencers, clog::space);
    } // guard

    size_t min_rank(std::numeric_limits<size_t>::max());
    std::set<size_t> shared_vertices;

    // Iterate the direct referencers to assign vertex ownership.
    for(auto c: referencers) {

      // Check the remote info map to see if this cell is
      // off-color. If it is, compare it's rank for
      // the ownership logic below.
      if(remote_info_map.find(c) != remote_info_map.end()) {
        min_rank = std::min(min_rank, remote_info_map[c].rank);
        shared_vertices.insert(remote_info_map[c].rank);
      }
      else {
        // If the referencing cell isn't in the remote info map
        // it is a local cell.

        // Add our rank to compare for ownership.
        min_rank = std::min(min_rank, size_t(rank));

        // If the local cell is shared, we need to add all of
        // the ranks that reference it.
        if(shared_cells_map.find(c) != shared_cells_map.end()) {
          shared_vertices.insert(shared_cells_map[c].shared.begin(),
            shared_cells_map[c].shared.end());
        } // if
      } // if

      // Iterate through the closure intersection map to see if the
      // indirect reference is part of another rank's closure, i.e.,
      // that it is an indirect dependency.
      for(auto ci: closure_intersection_map) {
        if(ci.second.find(c) != ci.second.end()) {
          shared_vertices.insert(ci.first);
        } // if
      } // for
    } // for

    if(min_rank == rank) {
      // This is a vertex that belongs to our rank.
      auto entry = entry_info_t(i, rank, offset, shared_vertices);
      vertex_info.insert(entry_info_t(i, rank, offset++, shared_vertices));
    }
    else {
      // Add remote vertex to the request for offset information.
      vertex_requests[min_rank].insert(i);
    } // if
  } // for

  auto vertex_offset_info =
    communicator->get_entity_info(vertex_info, vertex_requests);

  std::set<entry_info_t> exclusive_vertices;
  std::set<entry_info_t> shared_vertices;
  std::set<entry_info_t> ghost_vertices;

  for(auto i: vertex_info) {
    if(i.shared.size()) {
      shared_vertices.insert(i);
    }
    else {
      exclusive_vertices.insert(i);
    } // if
  } // for

  {
  size_t r(0);
  for(auto i: vertex_requests) {

    auto offset(vertex_offset_info[r].begin());
    for(auto s: i) {
      ghost_vertices.insert(entry_info_t(s, r, *offset));
      ++offset;
    } // for

    ++r;
  } // for
  } // scope

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "exclusive vertices ", exclusive_vertices,
    clog::newline);
  clog_container_one(info, "shared vertices ", shared_vertices, clog::newline);
  clog_container_one(info, "ghost vertices ", ghost_vertices, clog::newline);
  } // guard

  //----------------------------------------------------------------------------
  // output the result
  //----------------------------------------------------------------------------

  //------------------------------------
  // Open the file

  // figure out this ranks file name
  std::stringstream output_filename;
  output_filename << output_prefix;
  output_filename << "_rank";
  output_filename << std::setfill('0') << std::setw(6) << rank;
  output_filename << ".exo";

  // some type aliases
  using real_t = typename MD::real_t;
  using ex_real_t  = real_t;
  using ex_index_t = int;

  // size of floating point variables used in app.
  int app_word_size = sizeof(real_t);

  // size of floating point to be stored in file.
  // change to float to save space
  int exo_word_size = sizeof(ex_real_t);

  // determine the file creation mode
  int cmode = EX_CLOBBER;

  // create file
  auto exoid =
    ex_create(
      output_filename.str().c_str(), cmode, &app_word_size, &exo_word_size
    );
  if ( exoid < 0 ) 
    clog_fatal( 
      "Problem writing exodus file, ex_create() returned " << exoid
    );

  //------------------------------------
  // Set exodus parameters

  // set exodus parameters
  constexpr auto num_dims = MD::dimension();
  auto num_nodes = md.num_entities( 0 );
  auto num_elems = 
    exclusive_cells.size() + shared_cells.size() + ghost_cells.size();;

  ex_init_params exopar;
  strcpy( exopar.title, "Exodus II output from flecsi." );
  exopar.num_dim = num_dims;
  exopar.num_nodes = num_nodes;
  exopar.num_edge = 0;
  exopar.num_edge_blk = 0;
  exopar.num_face = 0; //num_faces;
  exopar.num_face_blk = 0; //1;
  exopar.num_elem = num_elems;
  exopar.num_elem_blk = 3; // excl, shared, ghost
  exopar.num_node_sets = 3; // excl, shared, ghost
  exopar.num_edge_sets = 0;
  exopar.num_face_sets = 0;
  exopar.num_side_sets = 0;
  exopar.num_elem_sets = 0;
  exopar.num_node_maps = 0;
  exopar.num_edge_maps = 0;
  exopar.num_face_maps = 0;
  exopar.num_elem_maps = 0;
  auto status = ex_put_init_ext( exoid, &exopar );
  assert(status == 0);
  
  //------------------------------------
  // Write the coordinates

  // write the coordiantes
  vector<real_t> vertex_coord( num_nodes * num_dims );
  
  for ( size_t i=0; i<num_nodes; ++i ) {
    const auto & vert = md.vertex(i);
    for ( int d=0; d<num_dims; ++d ) 
      vertex_coord[ d*num_nodes + i ] = vert[d];
  }

  status = ex_put_coord( 
    exoid, 
    vertex_coord.data(), 
    vertex_coord.data()+num_nodes, 
    vertex_coord.data()+2*num_nodes
  );
  if (status)
    clog_fatal(
      "Problem writing vertex coordinates to exodus file, " <<
      " ex_put_coord() returned " << status 
    );
      
  //------------------------------------
  // Lambda to write an element block
  auto write_elem_block = [&md]( 
    auto exoid, 
    auto elem_blk_id,
    const std::string & name,
    const auto & cell_list
  ) {

    // build the connectivitiy list for the exclusive cells
    vector<ex_index_t> elem_nodes;
    vector<ex_index_t> elem_node_counts;
    elem_nodes.reserve( cell_list.size() * (num_dims+1) );
    elem_node_counts.reserve( cell_list.size() );
    
    for ( auto c : cell_list ) {
      auto verts = md.vertices(num_dims, c.id);
      std::cout << c.id << " ";
      for ( auto v : verts ) 
        std::cout << v << " ";
      std::cout << std::endl;
      elem_node_counts.push_back( verts.size() );
      for (auto v: verts)
        elem_nodes.push_back( v + 1 ); // 1-based ids
    }

    // the total size needed to hold the element connectivity
    ex_index_t num_nodes_this_blk = elem_nodes.size();
    ex_index_t num_elems_this_blk = elem_node_counts.size();

    // set the block header
    ex_index_t num_attr_per_elem = 0;
    ex_index_t num_faces_per_elem = 0;
    ex_index_t num_edges_per_elem = 0;
    auto status = ex_put_block( 
      exoid, EX_ELEM_BLOCK, elem_blk_id, "nsided", num_elems_this_blk, 
      num_nodes_this_blk, num_edges_per_elem, num_faces_per_elem, 
      num_attr_per_elem
    );
    if (status)
      clog_fatal(
        "Problem writing element blocl to exodus file, " <<
        " ex_put_block() returned " << status 
      );

    // write the block name
    status = ex_put_name(
      exoid, EX_ELEM_BLOCK, elem_blk_id, name.c_str() 
    );
    if (status)
      clog_fatal(
        "Problem writing element block name to exodus file, " <<
        " ex_put_name() returned " << status 
      );

    // write connectivity
    status = ex_put_conn(
      exoid, EX_ELEM_BLOCK, elem_blk_id, elem_nodes.data(), 
      nullptr, nullptr
    );
    if (status)
      clog_fatal(
        "Problem writing element connectivity to exodus file, " <<
        " ex_put_conn() returned " << status 
      );
        
    // write counts
    status = ex_put_entity_count_per_polyhedra(
      exoid, EX_ELEM_BLOCK, elem_blk_id, elem_node_counts.data() 
    );
    if (status)
      clog_fatal(
        "Problem writing element counts to exodus file, " <<
        " ex_put_entity_count_per_polyhedra() returned " << status 
      );
  
  }; // write block
  
  //------------------------------------
  // A lambda to Write Node sets

  auto write_node_set = [&md]( 
    auto exoid,
    auto node_set_id,
    const std::string & name,
    const auto & vertex_list
  ) {
    
    // set the node set parameters
    ex_index_t num_dist_in_set = 0;
    ex_index_t num_nodes_this_set = vertex_list.size();
    auto status = ex_put_node_set_param(
      exoid, node_set_id, num_nodes_this_set, num_dist_in_set
    ); 
    if (status)
      clog_fatal(
        "Problem writing node set param to exodus file, " <<
        " ex_put_node_set_param() returned " << status 
      );

    // copy the vertex ids
    vector<ex_index_t> node_set;
    node_set.reserve( vertex_list.size() );
    
    for ( auto v : vertex_list ) 
      node_set.push_back( v.id );

    // write the node set
    status = ex_put_node_set(
      exoid, node_set_id, node_set.data()
    ); 
    if (status)
      clog_fatal(
        "Problem writing node set to exodus file, " <<
        " ex_put_node_set() returned " << status 
      );

    // write the set name
    status = ex_put_name(
      exoid, EX_NODE_SET, node_set_id, name.c_str() 
    );
    if (status)
      clog_fatal(
        "Problem writing node set name to exodus file, " <<
        " ex_put_name() returned " << status 
      );

  };


  //------------------------------------
  // Write Exclusive Cells / vertices
  
  // the block id and side set counter
  ex_index_t elem_blk_id = 0;
  ex_index_t node_set_id = 0;
  
  // write the cells
  write_elem_block( exoid, ++elem_blk_id, "exclusive cells", exclusive_cells );
  write_elem_block( exoid, ++elem_blk_id, "shared cells", shared_cells );
  write_elem_block( exoid, ++elem_blk_id, "ghost cells", ghost_cells );

  // write the vertices
  write_node_set( exoid, ++node_set_id, "exclusive vertices", exclusive_vertices );
  write_node_set( exoid, ++node_set_id, "shared vertices", shared_vertices );
  write_node_set( exoid, ++node_set_id, "ghost vertices", ghost_vertices );

      
  //------------------------------------
  // Close the file

  // close the file
  status = ex_close(exoid);
  if ( status ) 
    clog_fatal( 
      "Problem closing exodus file, ex_close() returned " << exoid 
    );

} // somerhing
  

DEVEL(coloring-unstruct)
{
  constexpr auto thru_dim = 0;

  auto prefix_2d = std::string( "exodus2d-mixed" );
  exodus_definition_2d_t md2d( prefix_2d+".exo" );
  color_cells<thru_dim>( md2d, prefix_2d+"-colored" );

  auto prefix_3d = std::string( "exodus3d-hex" );
  exodus_definition_3d_t md3d( prefix_3d+".exo");
  color_cells<thru_dim>( md3d, prefix_3d+"-colored" );
}

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
