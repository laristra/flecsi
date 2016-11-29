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

TEST(partition, simple) {

  using entry_info_t = flecsi::dmp::entry_info_t;

  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Create a mesh definition from file.
  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");

  // Create the dCRS representation for the distributed
  // partitioner.
  auto dcrs = flecsi::dmp::make_dcrs(sd);

  // Create a partitioner instance to generate the primary partition.
  auto partitioner = std::make_shared<flecsi::dmp::parmetis_partitioner_t>();

  // Create the primary partition.
  auto primary = partitioner->partition(dcrs);

  // Compute the dependency closure of the primary cell partition
  // through vertex intersections (specified by last argument "1").
  // To specify edge or face intersections, use 2 (edges) or 3 (faces).
  // FIXME: We may need to replace this with a predicate function.
  auto closure = flecsi::io::cell_closure(sd, primary, 1);

#if 0
  if(rank == 0 ) {
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

#if 0
  if(rank == 1) {
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

#if 0
  // Populate the shared and exclusive cell sets.
  for(size_t i(0); i<local_info.size(); ++i) {
      for(auto s: local_info[i]) {
        std::cout << s << " ";
      } // for
    if(local_info[i].size()) {
      shared_cells.insert(entry_info_t(primary_map[i], rank, i, local_info[i]));
    }
    else {
      exclusive_cells.insert(entry_info_t(primary_map[i], rank, i, {}));
    } // if
  } // for
#endif

#if 0
if(rank == 0) {
  std::cout << " cell_nn_info:" << std::endl;
  offset = 0;
  for(auto i: std::get<0>(cell_nn_info)) {
    std::cout << "index " << primary_indices_map[offset++] << ": ";
    for(auto v: i) {
      std::cout << v << " ";
    }
    std::cout << std::endl;
  } // for
} // if

  auto cell_all_info = partitioner->get_cell_info(primary, all_neighbors);

if(rank == 0) {
  std::cout << " cell_all_info:" << std::endl;
  offset = 0;
  for(auto i: std::get<0>(cell_all_info)) {
    std::cout << "index " << primary_indices_map[offset++] << ": ";
    for(auto v: i) {
      std::cout << v << " ";
    }
    std::cout << std::endl;
  } // for
} // if
#endif

#if 0
  // The union of the nearest and next-nearest neighbors gives us all
  // of the cells that might reference a vertex that we need.
  auto request_indices_set = flecsi::io::set_union(nearest_neighbors, nnn);

  // Get the rank and offset information for our cell dependencies.
  // This also gives back information about the ranks that access
  // our shared cells.
  auto cell_info = partitioner->get_cell_info(primary, request_indices_set);

  // This set contains information about shared indices for local cells.
  auto local_info = std::get<0>(cell_info);

  // This set contains entry info for remote cells.
  auto remote_info = std::get<1>(cell_info);

  if(rank == 0) {
  std::cout << "remote info: " << std::endl;
  for(auto i: remote_info) {
    std::cout << i << std::endl;
  } // for
  } // if

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, entry_info_t> remote_info_map;
  for(auto i: remote_info) {
    remote_info_map[i.id] = i;
  } // for

  if(rank == 0) {
    std::cout << " local_info:" << std::endl;
    offset = 0;
    for(auto i: local_info) {
      std::cout << "index: " << primary_indices_map[offset++] << " ";
      for(auto v: i) {
        std::cout << v << " ";
      }
      std::cout << std::endl;
    } // for
  } // if

  std::unordered_map<size_t, std::set<size_t>> local_info_map;
  offset = 0;
  if(rank == 0) {
  std::cout << "local_info_map: " << std::endl;
  for(auto i: local_info) {
    local_info_map[primary_indices_map[offset++]] = i;
    for(auto v: i) {
      std::cout << v << " ";
    } // for
    std::cout << std::endl;
  } // for
  } // if

  if(rank == 0) {
    std::cout << " local_info_map:" << std::endl;
    for(auto i: local_info_map) {
      for(auto v: i.second) {
      std::cout << v << " ";
      }
      std::cout << std::endl;
    } // for
  } // for

  std::set<size_t> shared_indices;
  if(rank == 0) {
  std::cout << "shared indices: " << std::endl;
  for(auto i: shared_indices) {
    std::cout << i << std::endl;
  } // for
  } // if

  std::set<entry_info_t> exclusive_cells;
  std::set<entry_info_t> shared_cells;
  std::set<entry_info_t> ghost_cells;

  std::unordered_map<size_t, entry_info_t> primary_map;

  // Create an index map for the primary cells set.
  size_t index(0);
  for(auto i: primary) {
    if(rank == 0) {
      std::cout << "adding map entry: " << i << " " << rank <<
        " " << index << std::endl;
    } // if
    primary_map[i] = entry_info_t(i, rank, index++, {});
  } // for

if(rank ==0) {
  for(auto i: primary) {
//#if 0
    auto match = local_info.insert(i);
    if(match.first) {
      std::cout << "primary " << i << " matched" << std::endl;
      for(auto v: match.second) {
        std::cout << v << " ";
      } // for
      std::cout << std::endl;
    } // if
//#endif
  } // for
} // if

//
// FIXME: Wrong...
//
  // Populate the shared and exclusive cell sets.
  for(size_t i(0); i<local_info.size(); ++i) {
      for(auto s: local_info[i]) {
        std::cout << s << " ";
      } // for
    if(local_info[i].size()) {
      shared_cells.insert(entry_info_t(primary_map[i], rank, i, local_info[i]));
    }
    else {
      exclusive_cells.insert(entry_info_t(primary_map[i], rank, i, {}));
    } // if
  } // for

  // Propulate the ghost cells set.
  for(auto n: nearest_neighbors) {
    ghost_cells.insert(remote_info_map[n]);
  } // for

if(rank == 0) {
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

  if(rank == 0) {
    for(auto i: remote_info) {
      std::cout << "cell " << i.id << " has rank " << i.rank <<
        " and offset " << i.offset << std::endl;
    } // for
  } // if

  // Form the vertex closure
  auto vertex_closure = flecsi::io::vertex_closure(sd, closure);

  // Assign vertex ownership
  std::vector<std::vector<size_t>> vertex_requests(size);
  std::set<entry_info_t> vertex_info;
  for(auto i: vertex_closure) {

    // Get the set of cells that reference this vertex.
    auto referencers = flecsi::io::vertex_referencers(sd, i);

//#if 0
    std::cout << "vertex " << i << " is referenced by cells: ";
    for(auto c: referencers) {
      std::cout << c << " ";
    } // for
    std::cout << std::endl;
//#endif

    size_t min_rank(std::numeric_limits<size_t>::max());
    std::vector<size_t> shared_vertices;

    for(auto c: referencers) {
      min_rank = std::min(min_rank, remote_info_map[c].rank);

      if(remote_info_map[c].rank != rank) {
        vertex_requests[min_rank].push_back(i);
        shared_vertices.push_back(rank);
      } // if
    } // for

//#if 0
    if(min_rank == rank) {
      vertex_info.insert({ i, rank, i, shared_vertices });
    }
    else {
      vertex_requests[min_rank].push_back(i);
    } // fi
//#endif

//#if 0
    std::cout << "vertex " << i << " belongs to rank " <<
      min_rank << std::endl;
//#endif
  } // for
#endif

// Still need to perform set operations to get shared vertices. Then,
// we need to exchange information with owner ranks to get the vertex
// offsets. All of this information should be collected into a
// vertex_info_t instance, similar to the entry_info_t type.

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
