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

#include <mpi.h>

#include "flecsi/io/simple_definition.h"
#include "flecsi/partition/dcrs_utils.h"
#include "flecsi/partition/parmetis_partitioner.h"
#include "flecsi/partition/mpi_communicator.h"
#include "flecsi/topology/graph_utils.h"
#include "flecsi/utils/set_utils.h"

clog_register_tag(partition);

DEVEL(partition) {

  // Set the output rank
  clog_set_output_rank(1);

  using entry_info_t = flecsi::dmp::entry_info_t;

  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Create a mesh definition from file.
#if 1
  const size_t M(8), N(8);
  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
#endif
#if 0
  const size_t M(16), N(16);
  flecsi::io::simple_definition_t sd("simple2d-16x16.msh");
#endif
#if 0
  const size_t M(32), N(32);
  flecsi::io::simple_definition_t sd("simple2d-32x32.msh");
#endif

  // Create the dCRS representation for the distributed partitioner.
  auto dcrs = flecsi::dmp::make_dcrs(sd);

  // Create a partitioner instance to generate the primary partition.
  auto partitioner = std::make_shared<flecsi::dmp::parmetis_partitioner_t>();

  // Create the primary partition.
  auto primary = partitioner->partition(dcrs);

  {
  clog_tag_guard(partition);
  clog_container_one(info, "primary partition", primary, clog::space);
  } // guard

  // Compute the dependency closure of the primary cell partition
  // through vertex intersections (specified by last argument "1").
  // To specify edge or face intersections, use 2 (edges) or 3 (faces).
  auto closure = flecsi::topology::entity_closure<2,2,0>(sd, primary);

  {
  clog_tag_guard(partition);
  clog_container_one(info, "closure", closure, clog::space);
  } // guard

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nearest_neighbors = flecsi::utils::set_difference(closure, primary);

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
  auto cell_nn_info = communicator->get_cell_info(primary, nearest_neighbors);

  //
  auto cell_all_info = communicator->get_cell_info(primary, all_neighbors);

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

  {
  clog_tag_guard(partition);
  clog_container_one(info, "exclusive cells ", exclusive_cells, clog::newline);
  clog_container_one(info, "shared cells ", shared_cells, clog::newline);
  clog_container_one(info, "ghost cells ", ghost_cells, clog::newline);
  } // guard

  // Create a map version for lookups below.
  std::unordered_map<size_t, entry_info_t> shared_cells_map;
  {
  for(auto i: shared_cells) {
    shared_cells_map[i.id] = i;
  } // for
  } // scope

  // Form the vertex closure
  auto vertex_closure = flecsi::topology::vertex_closure<2>(sd, closure);

  // Assign vertex ownership
  std::vector<std::set<size_t>> vertex_requests(size);
  std::set<entry_info_t> vertex_info;

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
      auto entry = entry_info_t(i, rank, offset, shared_vertices);
      vertex_info.insert(entry_info_t(i, rank, offset++, shared_vertices));
    }
    else {
      // Add remote vertex to the request for offset information.
      vertex_requests[min_rank].insert(i);
    } // fi
  } // for

  auto vertex_offset_info =
    communicator->get_vertex_info(vertex_info, vertex_requests);

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
  clog_tag_guard(partition);
  clog_container_one(info, "exclusive vertices ", exclusive_vertices,
    clog::newline);
  clog_container_one(info, "shared vertices ", shared_vertices, clog::newline);
  clog_container_one(info, "ghost vertices ", ghost_vertices, clog::newline);
  } // guard

#if 1
  std::vector<std::pair<std::string, std::string>> colors = {
    { "blue", "blue!40!white" },
    { "green!60!black", "green!60!white" },
    { "black", "black!40!white" },
    { "red", "red!40!white" },
    { "violet", "violet!40!white" }
  };

  std::stringstream texname;
  texname << "simple2d-" << rank << "-" << M << "x" << N << ".tex";
  std::ofstream tex(texname.str(), std::ofstream::out);

  tex << "% Mesh visualization" << std::endl;
  tex << "\\documentclass[tikz,border=7mm]{standalone}" << std::endl;
  tex << std::endl;

  tex << "\\begin{document}" << std::endl;
  tex << std::endl;

  tex << "\\begin{tikzpicture}" << std::endl;
  tex << std::endl;

  tex << "\\draw[step=1cm,black] (0, 0) grid (" <<
    M << ", " << N << ");" << std::endl;

  // maps
  std::unordered_map<size_t, entry_info_t> exclusive_cells_map;
  for(auto i: exclusive_cells) {
    exclusive_cells_map[i.id] = i;
  } // for

#if 0
  std::unordered_map<size_t, entry_info_t> shared_cells_map;
  for(auto i: shared_cells) {
    shared_cells_map[i.id] = i;
  } // for
#endif

  std::unordered_map<size_t, entry_info_t> ghost_cells_map;
  for(auto i: ghost_cells) {
    ghost_cells_map[i.id] = i;
  } // for

  size_t cell(0);
  for(size_t j(0); j<M; ++j) {
    double yoff(0.5+j);
    for(size_t i(0); i<M; ++i) {
      double xoff(0.5+i);

      // Cells
      auto ecell = exclusive_cells_map.find(cell);
      auto scell = shared_cells_map.find(cell);
      auto gcell = ghost_cells_map.find(cell);

      if(ecell != exclusive_cells_map.end()) {
        tex << "\\node[" << std::get<0>(colors[rank]) <<
          "] at (" << xoff << ", " << yoff <<
          ") {" << cell++ << "};" << std::endl;
      }
      else if(scell != shared_cells_map.end()) {
        tex << "\\node[" << std::get<1>(colors[rank]) <<
          "] at (" << xoff << ", " << yoff <<
          ") {" << cell++ << "};" << std::endl;
      }
      else if(gcell != ghost_cells_map.end()) {
        tex << "\\node[" << std::get<1>(colors[gcell->second.rank]) <<
          "] at (" << xoff << ", " << yoff <<
          ") {" << cell++ << "};" << std::endl;
      }
      else {
        tex << "\\node[white] at (" << xoff << ", " << yoff <<
          ") {" << cell++ << "};" << std::endl;
      } // if
    } // for
  } // for

  std::unordered_map<size_t, entry_info_t> exclusive_vertices_map;
  for(auto i: exclusive_vertices) {
    exclusive_vertices_map[i.id] = i;
  } // for

  std::unordered_map<size_t, entry_info_t> shared_vertices_map;
  for(auto i: shared_vertices) {
    shared_vertices_map[i.id] = i;
  } // for

  std::unordered_map<size_t, entry_info_t> ghost_vertices_map;
  for(auto i: ghost_vertices) {
    ghost_vertices_map[i.id] = i;
  } // for

  size_t vertex(0);
  for(size_t j(0); j<M+1; ++j) {
    double yoff(j-0.15);
    for(size_t i(0); i<N+1; ++i) {
      double xoff(i-0.2);

      auto evertex = exclusive_vertices_map.find(vertex);
      auto svertex = shared_vertices_map.find(vertex);
      auto gvertex = ghost_vertices_map.find(vertex);

      if(evertex != exclusive_vertices_map.end()) {
        tex << "\\node[" << std::get<0>(colors[rank]) <<
          "] at (" << xoff << ", " << yoff <<
          ") { \\scriptsize " << vertex++ << "};" << std::endl;
      }
      else if(svertex != shared_vertices_map.end()) {
        tex << "\\node[" << std::get<1>(colors[rank]) <<
          "] at (" << xoff << ", " << yoff <<
          ") { \\scriptsize " << vertex++ << "};" << std::endl;
      }
      else if(gvertex != ghost_vertices_map.end()) {
        tex << "\\node[" << std::get<1>(colors[gvertex->second.rank]) <<
          "] at (" << xoff << ", " << yoff <<
          ") { \\scriptsize " << vertex++ << "};" << std::endl;
      }
      else {
        tex << "\\node[white] at (" << xoff << ", " << yoff <<
          ") { \\scriptsize " << vertex++ << "};" << std::endl;
      } // if
    } // for
  } // for

  tex << "\\end{tikzpicture}" << std::endl;
  tex << std::endl;

  tex << "\\end{document}" << std::endl;
#endif

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
