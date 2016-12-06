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

#include <cinchtest.h>

#include <mpi.h>

#include "flecsi/io/set_utils.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/partition/dcrs_utils.h"
#include "flecsi/partition/parmetis_partitioner.h"

const size_t output_rank = 1;

TEST(partition, simple) {

  using entry_info_t = flecsi::dmp::entry_info_t;

  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Create a mesh definition from file.
  //flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  //flecsi::io::simple_definition_t sd("simple2d-1024x1024.msh");
  //flecsi::io::simple_definition_t sd("simple2d-100x100.msh");
  //flecsi::io::simple_definition_t sd("simple2d-21x21.msh");
  //flecsi::io::simple_definition_t sd("simple2d-32x32.msh");
  flecsi::io::simple_definition_t sd("simple2d-16x16.msh");
  //flecsi::io::simple_definition_t sd("simple2d-10x10.msh");

  // Create the dCRS representation for the distributed
  // partitioner.
  auto dcrs = flecsi::dmp::make_dcrs(sd);

  // Create a partitioner instance to generate the primary partition.
  auto partitioner = std::make_shared<flecsi::dmp::parmetis_partitioner_t>();

  // Create the primary partition.
  auto primary = partitioner->partition(dcrs);

#if 0
  if(rank == output_rank) {
    std::cout << "primary: ";
    for(auto i: primary) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;
  } // if
#endif

  // Compute the dependency closure of the primary cell partition
  // through vertex intersections (specified by last argument "1").
  // To specify edge or face intersections, use 2 (edges) or 3 (faces).
  // FIXME: We may need to replace this with a predicate function.
  auto closure = flecsi::io::cell_closure(sd, primary, 1);

#if 0
  if(rank == output_rank) {
    std::cout << "closure:" << std::endl;
    for(auto i: closure) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;
  } // if
#endif

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nearest_neighbors = flecsi::io::set_difference(closure, primary);

  // The closure of the nearest neighbors intersected with
  // the initial indeces gives the shared indices. This is similar to
  // the preimage of the nearest neighbors.
  auto nearest_neighbor_closure =
    flecsi::io::cell_closure(sd, nearest_neighbors, 1);

#if 0
  auto shared = flecsi::io::set_intersection(nearest_neighbor_closure, primary);
  auto exclusive = flecsi::io::set_difference(primary, shared);
#endif

  // We can iteratively add halos of nearest neighbors, e.g.,
  // here we add the next nearest neighbors. For most mesh types
  // we actually need information about the ownership of these indices
  // so that we can deterministically assign rank ownership to vertices.
  auto next_nearest_neighbors =
    flecsi::io::set_difference(nearest_neighbor_closure, closure);

  // The union of the nearest and next-nearest neighbors gives us all
  // of the cells that might reference a vertex that we need.
  auto all_neighbors = flecsi::io::set_union(nearest_neighbors,
    next_nearest_neighbors);

  // Get the rank and offset information for our nearest neighbor
  // dependencies. This also gives information about the ranks
  // that access our shared cells.
  auto cell_nn_info = partitioner->get_cell_info(primary, nearest_neighbors);

  //
  auto cell_all_info = partitioner->get_cell_info(primary, all_neighbors);

  // Create a map version of the local info for lookups below.
  std::unordered_map<size_t, size_t> primary_indices_map;
  {
  size_t offset(0);
  for(auto i: primary) {
    primary_indices_map[offset++] = i;
  } // for
  } // scope

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, entry_info_t> remote_info_map;
  for(auto i: std::get<1>(cell_all_info)) {
    remote_info_map[i.id] = i;
  } // for

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

#if 1
  if(rank == output_rank) {
    std::cout << "exclusive cells: " << std::endl;
    for(auto i: exclusive_cells) {
      std::cout << i << std::endl;
    } // for
    std::cout << "shared cells: " << std::endl;
    for(auto i: shared_cells) {
      std::cout << i << std::endl;
    } // for
    std::cout << "ghost cells: " << std::endl;
    for(auto i: ghost_cells) {
      std::cout << i << std::endl;
    } // for
  } // if
#endif

// FIXME
#if 1
  // Create a map version for lookups below.
  std::unordered_map<size_t, entry_info_t> shared_cells_map;
  {
  for(auto i: shared_cells) {
    shared_cells_map[i.id] = i;
  } // for
  } // scope
#endif

  // Form the vertex closure
  auto vertex_closure = flecsi::io::vertex_closure(sd, closure);

#if 0
  std::cout << "rank " << rank << " vertex closure: " << std::endl;
  for(auto i: vertex_closure) {
    std::cout << i << " ";
  } // for
  std::cout << std::endl;
#endif

  // Assign vertex ownership
  std::vector<std::set<size_t>> vertex_requests(size);
  std::set<entry_info_t> vertex_info;

  size_t offset(0);
  for(auto i: vertex_closure) {

    // Get the set of cells that reference this vertex.
    auto referencers = flecsi::io::vertex_referencers(sd, i);

#if 0
  if(rank == output_rank) {
    std::cout << "vertex " << i << " is referenced by cells: ";
    for(auto c: referencers) {
      std::cout << c << " ";
    } // for
    std::cout << std::endl;
  } // if
#endif

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
      auto entry = entry_info_t(i, rank, offset, shared_vertices);
      vertex_info.insert(entry_info_t(i, rank, offset++, shared_vertices));
    }
    else {
      // Add remote vertex to the request for offset information.
      vertex_requests[min_rank].insert(i);
    } // fi
  } // for

#if 0
  if(rank == output_rank) {
    std::cout << "vertex info" << std::endl;
    for(auto i: vertex_info) {
      std::cout << i << std::endl;
    } // for
  } // if

  if(rank == output_rank) {
    std::cout << "rank " << rank << " vertex requests" << std::endl;
    size_t r(0);
    for(auto i: vertex_requests) {
      std::cout << "rank " << r++ << std::endl;
      for(auto s: i) {
        std::cout << s << " ";
      } // for
      std::cout << std::endl;
    } // for
  } // if
#endif

  auto vertex_offset_info =
    partitioner->get_vertex_info(vertex_info, vertex_requests);

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

if(rank == output_rank) {
  std::cout << "exclusive vertices:" << std::endl;
  for(auto i: exclusive_vertices) {
    std::cout << i << std::endl;
  } // for

  std::cout << "shared vertices:" << std::endl;
  for(auto i: shared_vertices) {
    std::cout << i << std::endl;
  } // for

  std::cout << "ghost vertices:" << std::endl;
  for(auto i: ghost_vertices) {
    std::cout << i << std::endl;
  } // for
} // if

// We will need to discuss how to expose customization to user
// specializations. The particular configuration of ParMETIS is
// likley to need tweaks. There are also likely other closure
// definitions than the one that was chosen here.

// So far, I am happy that this is relatively concise.
} // TEST

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
