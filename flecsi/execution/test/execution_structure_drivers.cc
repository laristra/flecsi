/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include <cinchlog.h>

#include "flecsi/execution/execution.h"
#include "flecsi/execution/context.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/partition/communicator.h"
#include "flecsi/partition/dcrs_utils.h"
#include "flecsi/partition/parmetis_partitioner.h"
#include "flecsi/partition/mpi_communicator.h"
#include "flecsi/topology/graph_utils.h"
#include "flecsi/utils/set_utils.h"

clog_register_tag(partition);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Create some basic partitions.
//----------------------------------------------------------------------------//

void add_partitions(int dummy) {

  clog_set_output_rank(0);

  // Get the context instance.
  context_t & context_ = context_t::instance();

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  clog(info) << "add_partitions, rank: " << rank << std::endl;

  // Read the mesh definition from file.
  //flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  flecsi::io::simple_definition_t sd("simple2d-16x16.msh");

  // Create the dCRS representation for the distributed partitioner.
  auto dcrs = flecsi::dmp::make_dcrs(sd);

  // Create a partitioner instance to generate the primary partition.
  auto partitioner = std::make_shared<flecsi::dmp::parmetis_partitioner_t>();

  // Cells index partition.
  flecsi::dmp::index_partition_t cells;

  // Create the primary partition.
  cells.primary = partitioner->partition(dcrs);

  {
  clog_tag_guard(partition);
  clog_container_one(info, "primary partition", cells.primary, clog::space);
  } // guard

  // Compute the dependency closure of the primary cell partition
  // through vertex intersections (specified by last argument "1").
  // To specify edge or face intersections, use 2 (edges) or 3 (faces).
  auto closure = flecsi::topology::entity_closure<2,2,0>(sd, cells.primary);

  {
  clog_tag_guard(partition);
  clog_container_one(info, "closure", closure, clog::space);
  } // guard

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nearest_neighbors =
    flecsi::utils::set_difference(closure, cells.primary);

  {
  clog_tag_guard(partition);
  clog_container_one(info, "nearest neighbors", nearest_neighbors, clog::space);
  } // guard

  // We can iteratively add halos of nearest neighbors, e.g.,
  // here we add the next nearest neighbors. For most mesh types
  // we actually need information about the ownership of these indices
  // so that we can deterministically assign rank ownership to vertices.
  auto nearest_neighbor_closure =
    flecsi::topology::entity_closure<2,2,0>(sd, nearest_neighbors);

  {
  clog_tag_guard(partition);
  clog_container_one(info, "nearest neighbor closure",
    nearest_neighbor_closure, clog::space);
  } // guard

  // Subtracting out the closure leaves just the
  // next nearest neighbors.
  auto next_nearest_neighbors =
    flecsi::utils::set_difference(nearest_neighbor_closure, closure);

  {
  clog_tag_guard(partition);
  clog_container_one(info, "next nearest neighbor", next_nearest_neighbors,
    clog::space);
  } // guard

  // The union of the nearest and next-nearest neighbors gives us all
  // of the cells that might reference a vertex that we need.
  auto all_neighbors = flecsi::utils::set_union(nearest_neighbors,
    next_nearest_neighbors);

  {
  clog_tag_guard(partition);
  clog_container_one(info, "all neighbors", all_neighbors, clog::space);
  } // guard

  // Create a communicator instance to get neighbor information.
  auto communicator = std::make_shared<flecsi::dmp::mpi_communicator_t>();

  // Get the rank and offset information for our nearest neighbor
  // dependencies. This also gives information about the ranks
  // that access our shared cells.
  auto cell_nn_info =
    communicator->get_cell_info(cells.primary, nearest_neighbors);

  //
  auto cell_all_info =
    communicator->get_cell_info(cells.primary, all_neighbors);

  // Create a map version of the local info for lookups below.
  std::unordered_map<size_t, size_t> primary_indices_map;
  {
  size_t offset(0);
  for(auto i: cells.primary) {
    primary_indices_map[offset++] = i;
  } // for
  } // scope

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, flecsi::dmp::entry_info_t> remote_info_map;
  for(auto i: std::get<1>(cell_all_info)) {
    remote_info_map[i.id] = i;
  } // for

  // Populate exclusive and shared cell information.
  {
  size_t offset(0);
  for(auto i: std::get<0>(cell_nn_info)) {
    if(i.size()) {
      cells.shared.insert(
        flecsi::dmp::entry_info_t(primary_indices_map[offset],
        rank, offset, i));
    }
    else {
      cells.exclusive.insert(
        flecsi::dmp::entry_info_t(primary_indices_map[offset],
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
  } // for
  } // scope

  {
  clog_tag_guard(partition);
  clog_container_one(info, "exclusive cells ", cells.exclusive, clog::newline);
  clog_container_one(info, "shared cells ", cells.shared, clog::newline);
  clog_container_one(info, "ghost cells ", cells.ghost, clog::newline);
  } // guard

  // Create a map version for lookups below.
  std::unordered_map<size_t, flecsi::dmp::entry_info_t> shared_cells_map;
  {
  for(auto i: cells.shared) {
    shared_cells_map[i.id] = i;
  } // for
  } // scope

  // Form the vertex closure
  auto vertex_closure = flecsi::topology::vertex_closure<2>(sd, closure);

  // Assign vertex ownership
  std::vector<std::set<size_t>> vertex_requests(size);
  std::set<flecsi::dmp::entry_info_t> vertex_info;

  size_t offset(0);
  for(auto i: vertex_closure) {

    // Get the set of cells that reference this vertex.
    auto referencers = flecsi::topology::vertex_referencers<2>(sd, i);

    size_t min_rank(std::numeric_limits<size_t>::max());
    std::set<size_t> shared_vertices;

    for(auto c: referencers) {

      // If the referencing cell isn't in the remote info map
      // it is a local cell.
      if(remote_info_map.find(c) != remote_info_map.end()) {
        min_rank = std::min(min_rank, remote_info_map[c].rank);
        shared_vertices.insert(remote_info_map[c].rank);
      }
      else {
        min_rank = std::min(min_rank, size_t(rank));

        // If the local cell is shared, we need to add all of
        // the ranks that reference it.
        if(shared_cells_map.find(c) != shared_cells_map.end()) {
          shared_vertices.insert(shared_cells_map[c].shared.begin(),
            shared_cells_map[c].shared.end());
        } // if
      } // if
    } // for

    if(min_rank == rank) {
      // This is a vertex that belongs to our rank.
      auto entry = flecsi::dmp::entry_info_t(i, rank, offset, shared_vertices);
      vertex_info.insert(
        flecsi::dmp::entry_info_t(i, rank, offset++, shared_vertices));
    }
    else {
      // Add remote vertex to the request for offset information.
      vertex_requests[min_rank].insert(i);
    } // fi
  } // for

  auto vertex_offset_info =
    communicator->get_vertex_info(vertex_info, vertex_requests);

  // Vertices index partition.
  flecsi::dmp::index_partition_t vertices;

  for(auto i: vertex_info) {
    if(i.shared.size()) {
      vertices.shared.insert(i);
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
      vertices.ghost.insert(flecsi::dmp::entry_info_t(s, r, *offset));
      ++offset;
    } // for

    ++r;
  } // for
  } // scope

  {
  clog_tag_guard(partition);
  clog_container_one(info, "exclusive vertices ", vertices.exclusive,
    clog::newline);
  clog_container_one(info, "shared vertices ", vertices.shared, clog::newline);
  clog_container_one(info, "ghost vertices ", vertices.ghost, clog::newline);
  } // guard

  // Add partitions to the context.
  context_.add_partition(0, cells);
  context_.add_partition(1, vertices);

} // add_partitions


flecsi_register_task(add_partitions, mpi, index);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_driver(int argc, char ** argv) {
  clog(info) << "In specialization driver" << std::endl;

  flecsi_execute_task(add_partitions, mpi, index, 0);

} // specialization_driver

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(info) << "In driver" << std::endl;
} // specialization_driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
