/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include <cinchtest.h>

#include "flecsi/execution/execution.h"
#include "flecsi/execution/context.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/coloring/coloring_types.h"
#include "flecsi/coloring/communicator.h"
#include "flecsi/coloring/dcrs_utils.h"
#include "flecsi/coloring/parmetis_colorer.h"
#include "flecsi/coloring/mpi_communicator.h"
#include "flecsi/topology/closure_utils.h"
#include "flecsi/utils/set_utils.h"
#include "flecsi/data/data.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

using namespace flecsi;

clog_register_tag(coloring);

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t = 
  data::legion::dense_handle_t<T, EP, SP, GP,
  data::legion_meta_data_t<default_user_meta_data_t>>;

void task1(handle_t<double, dro, dno, dno> x, int y) {
  np(y);
} // task1

flecsi_register_task(task1, loc, single);

class client_type : public flecsi::data::data_client_t{};

flecsi_new_register_data(client_type, ns, pressure, double, dense, 0, 1);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Create some basic colorings.
//----------------------------------------------------------------------------//

void add_colorings(int dummy) {

  clog_set_output_rank(0);

  // Get the context instance.
  context_t & context_ = context_t::instance();

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  clog(info) << "add_colorings, rank: " << rank << std::endl;

  // Read the mesh definition from file.
  //flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  flecsi::io::simple_definition_t sd("simple2d-16x16.msh");

  // Create the dCRS representation for the distributed colorer.
  auto dcrs = flecsi::coloring::make_dcrs(sd);

  // Create a colorer instance to generate the primary coloring.
  auto colorer = std::make_shared<flecsi::coloring::parmetis_colorer_t>();

  // Cells index coloring.
  flecsi::coloring::index_coloring_t cells;
  flecsi::coloring::coloring_info_t cell_color_info;
 
  // Create the primary coloring.
  cells.primary = colorer->color(dcrs);

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "primary coloring", cells.primary, clog::space);
  } // guard

  // Compute the dependency closure of the primary cell coloring
  // through vertex intersections (specified by last argument "0").
  // To specify edge or face intersections, use 1 (edges) or 2 (faces).
  auto closure = flecsi::topology::entity_closure<2,2,0>(sd, cells.primary);

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "closure", closure, clog::space);
  } // guard

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nearest_neighbors =
    flecsi::utils::set_difference(closure, cells.primary);

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "nearest neighbors", nearest_neighbors, clog::space);
  } // guard

  // Create a communicator instance to get neighbor information.
  auto communicator = std::make_shared<flecsi::coloring::mpi_communicator_t>();

  // Get the intersection of our nearest neighbors with the nearest
  // neighbors of other ranks. This map of sets will only be populated
  // with intersections that are non-empty
  auto closure_intersection_map =
    communicator->get_intersection_info(nearest_neighbors);

  // We can iteratively add halos of nearest neighbors, e.g.,
  // here we add the next nearest neighbors. For most mesh types
  // we actually need information about the ownership of these indices
  // so that we can deterministically assign rank ownership to vertices.
  auto nearest_neighbor_closure =
    flecsi::topology::entity_closure<2,2,0>(sd, nearest_neighbors);

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

  // Get the rank and offset information for our nearest neighbor
  // dependencies. This also gives information about the ranks
  // that access our shared cells.
  auto cell_nn_info =
    communicator->get_primary_info(cells.primary, nearest_neighbors);

  // Get the rank and offset information for all relevant neighbor
  // dependencies. This information will be necessary for determining
  // shared vertices.
  auto cell_all_info =
    communicator->get_primary_info(cells.primary, all_neighbors);

  // Create a map version of the local info for lookups below.
  std::unordered_map<size_t, size_t> primary_indices_map;
  {
  size_t offset(0);
  for(auto i: cells.primary) {
    primary_indices_map[offset++] = i;
  } // for
  } // scope

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, flecsi::coloring::entity_info_t> remote_info_map;
  for(auto i: std::get<1>(cell_all_info)) {
    remote_info_map[i.id] = i;
  } // for

  // Populate exclusive and shared cell information.
  {
  size_t offset(0);
  for(auto i: std::get<0>(cell_nn_info)) {
    if(i.size()) {
      cells.shared.insert(
        flecsi::coloring::entity_info_t(primary_indices_map[offset],
        rank, offset, i));
      
      // Collect all colors with whom we require communication
      // to send shared information.
      cell_color_info.shared_users = flecsi::utils::set_union(
        cell_color_info.shared_users, i);
    }
    else {
      cells.exclusive.insert(
        flecsi::coloring::entity_info_t(primary_indices_map[offset],
        rank, offset, i));
    } // if
    ++offset;
  } // for
  } // scope
  
  // Populate ghost cell information.
  {
  size_t offset(0);
  for(auto i: std::get<1>(cell_nn_info)) {
    cells.ghost.insert(i);

    // Collect all colors with whom we require communication
    // to receive ghost information.
    cell_color_info.ghost_owners.insert(i.rank);
  } // for
  } // scope

  cell_color_info.exclusive = cells.exclusive.size();
  cell_color_info.shared = cells.shared.size();
  cell_color_info.ghost = cells.ghost.size();

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "exclusive cells ", cells.exclusive, clog::newline);
  clog_container_one(info, "shared cells ", cells.shared, clog::newline);
  clog_container_one(info, "ghost cells ", cells.ghost, clog::newline);
  } // guard

  // Create a map version for lookups below.
  std::unordered_map<size_t, flecsi::coloring::entity_info_t> shared_cells_map;
  {
  for(auto i: cells.shared) {
    shared_cells_map[i.id] = i;
  } // for
  } // scope

  // Form the vertex closure
  auto vertex_closure = flecsi::topology::vertex_closure<2>(sd, closure);

  // Assign vertex ownership
  std::vector<std::set<size_t>> vertex_requests(size);
  std::set<flecsi::coloring::entity_info_t> vertex_info;

  size_t offset(0);
  for(auto i: vertex_closure) {

    // Get the set of cells that reference this vertex.
    auto referencers = flecsi::topology::vertex_referencers<2>(sd, i);

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
        // If the local cell is shared, we need to add all of
        // the ranks that reference it.

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
      auto entry =
        flecsi::coloring::entity_info_t(i, rank, offset, shared_vertices);
      vertex_info.insert(
        flecsi::coloring::entity_info_t(i, rank, offset++, shared_vertices));
    }
    else {
      // Add remote vertex to the request for offset information.
      vertex_requests[min_rank].insert(i);
    } // fi
  } // for

  auto vertex_offset_info =
    communicator->get_entity_info(vertex_info, vertex_requests);

  // Vertices index coloring.
  flecsi::coloring::index_coloring_t vertices;
  coloring::coloring_info_t vertex_color_info;

  for(auto i: vertex_info) {
    if(i.shared.size()) {
      vertices.shared.insert(i);

      // Collect all colors with whom we require communication
      // to send shared information.
      vertex_color_info.shared_users = flecsi::utils::set_union(
        vertex_color_info.shared_users, i.shared);
    }
    else {
      vertices.exclusive.insert(i);
    } // if
  } // for

  {
  size_t r(0);
  for(auto i: vertex_requests) {

    auto offset(vertex_offset_info[r].begin());
    for(auto s: i) {
      vertices.ghost.insert(flecsi::coloring::entity_info_t(s, r, *offset));
      ++offset;

      // Collect all colors with whom we require communication
      // to receive ghost information.
      vertex_color_info.ghost_owners.insert(r);
    } // for

    ++r;
  } // for
  } // scope

  {
  clog_tag_guard(coloring);
  clog_container_one(info, "exclusive vertices ", vertices.exclusive,
    clog::newline);
  clog_container_one(info, "shared vertices ", vertices.shared, clog::newline);
  clog_container_one(info, "ghost vertices ", vertices.ghost, clog::newline);
  } // guard

#if 0
  coloring::coloring_info_t cell_color_info{ cells.exclusive.size(),
    cells.shared.size(), cells.ghost.size() };
  coloring::coloring_info_t vertex_color_info{ vertices.exclusive.size(),
    vertices.shared.size(), vertices.ghost.size() };
#endif

  vertex_color_info.exclusive = vertices.exclusive.size();
  vertex_color_info.shared = vertices.shared.size();
  vertex_color_info.ghost = vertices.ghost.size();

  {
  clog_tag_guard(coloring);
  clog(info) << cell_color_info << std::endl << std::flush;
  clog(info) << vertex_color_info << std::endl << std::flush;
  } // gaurd

  // Gather the coloring info from all colors
  auto cell_coloring_info = communicator->get_coloring_info(cell_color_info);
  auto vertex_coloring_info =
    communicator->get_coloring_info(vertex_color_info);

  // Add colorings to the context.
  context_.add_coloring(0, cells, cell_coloring_info);
  context_.add_coloring(1, vertices, vertex_coloring_info);

} // add_colorings

flecsi_register_mpi_task(add_colorings);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_driver(int argc, char ** argv) {
  clog(info) << "In specialization driver" << std::endl;
  flecsi_execute_mpi_task(add_colorings, 0);

} // specialization_driver

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(info) << "In driver" << std::endl;

  client_type c;

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto h = flecsi_get_handle(c, ns, pressure, double, dense, 0);

  flecsi_execute_task(task1, single, h, 128);
} // specialization_driver

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

TEST(execution_structure, testname) {
  
} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
