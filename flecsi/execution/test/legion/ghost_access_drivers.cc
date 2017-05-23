/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: May 4, 2017
///

#include <cinchlog.h>
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

clog_register_tag(coloring);

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

  {
  clog_tag_guard(coloring);
  clog(info) << cell_color_info << std::endl << std::flush;
  } // gaurd

  // Gather the coloring info from all colors
  auto cell_coloring_info = communicator->get_coloring_info(cell_color_info);

  // Add colorings to the context.
  context_.add_coloring(0, cells, cell_coloring_info);

} // add_colorings

flecsi_register_mpi_task(add_colorings);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_driver(int argc, char ** argv) {
  clog(trace) << "In specialization driver" << std::endl;

  flecsi_execute_mpi_task(add_colorings, 0);

} // specialization_driver

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(trace) << "In driver" << std::endl;
} // specialization_driver

} // namespace execution
} // namespace flecsi


TEST(ghost_access, testname) {

} // TEST


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
