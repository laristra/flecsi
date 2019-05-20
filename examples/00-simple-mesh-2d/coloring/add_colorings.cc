/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#include <iostream>
#include <vector> 

#include <mpi.h>

#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/coloring/mpi_communicator.h>
#include <flecsi/coloring/parmetis_colorer.h>
#include <flecsi/execution/execution.h>
#include <flecsi/io/simple_definition.h>

#include "add_colorings.h"
#include "coloring_functions.h"

namespace flecsi {
namespace simple_mesh_coloring {

using namespace execution;

void
add_colorings(coloring_map_t map) {

  // Get the context instance.
  context_t & context_ = context_t::instance();

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Read the mesh definition from file.
  const size_t M(16), N(16);
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

  // Compute the dependency closure of the primary cell coloring
  // through vertex intersections (specified by last argument "0").
  // To specify edge or face intersections, use 1 (edges) or 2 (faces).
  auto closure = flecsi::topology::entity_neighbors<2, 2, 0>(sd, cells.primary);

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nearest_neighbors =
    flecsi::utils::set_difference(closure, cells.primary);

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
    flecsi::topology::entity_neighbors<2, 2, 0>(sd, nearest_neighbors);

  // Subtracting out the closure leaves just the
  // next nearest neighbors.
  auto next_nearest_neighbors =
    flecsi::utils::set_difference(nearest_neighbor_closure, closure);

  // The union of the nearest and next-nearest neighbors gives us all
  // of the cells that might reference a vertex that we need.
  auto all_neighbors =
    flecsi::utils::set_union(nearest_neighbors, next_nearest_neighbors);

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
    for(auto i : cells.primary) {
      primary_indices_map[offset++] = i;
    } // for
  } // scope

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, flecsi::coloring::entity_info_t> remote_info_map;
  for(auto i : std::get<1>(cell_all_info)) {
    remote_info_map[i.id] = i;
  } // for

  // Populate exclusive and shared cell information.
  {
    size_t offset(0);
    for(auto i : std::get<0>(cell_nn_info)) {
      if(i.size()) {
        cells.shared.insert(flecsi::coloring::entity_info_t(
          primary_indices_map[offset], rank, offset, i));

        // Collect all colors with whom we require communication
        // to send shared information.
        cell_color_info.shared_users =
          flecsi::utils::set_union(cell_color_info.shared_users, i);
      }
      else {
        cells.exclusive.insert(flecsi::coloring::entity_info_t(
          primary_indices_map[offset], rank, offset, i));
      } // if
      ++offset;
    } // for
  } // scope

  // Populate ghost cell information.
  {
    size_t offset(0);
    for(auto i : std::get<1>(cell_nn_info)) {
      cells.ghost.insert(i);

      // Collect all colors with whom we require communication
      // to receive ghost information.
      cell_color_info.ghost_owners.insert(i.rank);
    } // for
  } // scope

  cell_color_info.exclusive = cells.exclusive.size();
  cell_color_info.shared = cells.shared.size();
  cell_color_info.ghost = cells.ghost.size();

  // Create a map version for lookups below.
  std::unordered_map<size_t, flecsi::coloring::entity_info_t> shared_cells_map;
  {
    for(auto i : cells.shared) {
      shared_cells_map[i.id] = i;
    } // for
  } // scope

  //--------------------------------------------------------------------------//
  //--------------------------------------------------------------------------//

  flecsi::coloring::index_coloring_t vertices;
  coloring::coloring_info_t vertex_color_info;

  color_entity<2, 0>(sd, communicator.get(), closure, remote_info_map,
    shared_cells_map, closure_intersection_map, vertices, vertex_color_info);

  // Gather the coloring info from all colors
  auto cell_coloring_info = communicator->gather_coloring_info(cell_color_info);
  auto vertex_coloring_info =
    communicator->gather_coloring_info(vertex_color_info);

  // Add colorings to the context.
  context_.add_coloring(map.cells, cells, cell_coloring_info);
  context_.add_coloring(map.vertices, vertices, vertex_coloring_info);

  // Maps for output
  std::unordered_map<size_t, flecsi::coloring::entity_info_t>
    exclusive_cells_map;
  for(auto i : cells.exclusive) {
    exclusive_cells_map[i.id] = i;
  } // for

  std::unordered_map<size_t, flecsi::coloring::entity_info_t> ghost_cells_map;
  for(auto i : cells.ghost) {
    ghost_cells_map[i.id] = i;
  } // for

  std::unordered_map<size_t, flecsi::coloring::entity_info_t>
    exclusive_vertices_map;
  for(auto i : vertices.exclusive) {
    exclusive_vertices_map[i.id] = i;
    vertices.primary.insert(i.id);
  } // for

  std::unordered_map<size_t, flecsi::coloring::entity_info_t>
    shared_vertices_map;
  for(auto i : vertices.shared) {
    shared_vertices_map[i.id] = i;
    vertices.primary.insert(i.id);
  } // for

  std::unordered_map<size_t, flecsi::coloring::entity_info_t>
    ghost_vertices_map;
  for(auto i : vertices.ghost) {
    ghost_vertices_map[i.id] = i;
  } // for

  // Gather primary partitions
  auto primary_cells = communicator->get_entity_reduction(cells.primary);
  auto primary_vertices = communicator->get_entity_reduction(vertices.primary);
  
} // add_colorings

flecsi_register_mpi_task(add_colorings, flecsi::simple_mesh_coloring);

} // namespace simple_mesh_coloring
} // namespace flecsi
