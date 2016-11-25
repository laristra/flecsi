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
  // We may need to replace this with a predicate function.
  auto closure = flecsi::io::cell_closure(sd, primary, 1);

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nn = flecsi::io::set_difference(closure, primary);

  // The closure of the nearest neighbors intersected with
  // the initial indeces gives the shared indices. This is similar to
  // the preimage of the nearest neighbors.
  auto nnclosure = flecsi::io::cell_closure(sd, nn, 1);
  auto shared = flecsi::io::set_intersection(nnclosure, primary);

  // We can iteratively add halos of nearest neighbors, e.g.,
  // here we add the next nearest neighbors. For most mesh types
	// we actually need information about the ownership of these indices
	// so that we can deterministically assign rank ownership to vertices.
  auto nnn = flecsi::io::set_difference(flecsi::io::cell_closure(sd, nn, 1),
    closure);

	// The union of the nearest and next-nearest neighbors gives us all
	// of the cells that might reference a vertex that we need.
	auto request_indices_set = flecsi::io::set_union(nn, nnn);

  auto cell_info = partitioner->get_cell_info(primary, request_indices_set);
  
  // Create a map version of the cell info for lookups below.
  std::unordered_map<size_t, flecsi::dmp::cell_info_t> cell_info_map;
  for(auto i: cell_info) {
    cell_info_map[i.id] = i;
  } // for

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank == 0) {
    for(auto i: cell_info) {
      std::cout << "cell " << i.id << " has rank " << i.rank <<
        " and offset " << i.offset << std::endl;
    } // for

    // Form the vertex closure
    auto vertex_closure = flecsi::io::vertex_closure(sd, closure);

    // Assign vertex ownership
    for(auto i: vertex_closure) {

      // Get the set of cells that reference this vertex.
      auto referencers = flecsi::io::vertex_referencers(sd, i);

      std::cout << "vertex " << i << " is referenced by cells: ";
      for(auto c: referencers) {
        std::cout << c << " ";
      } // for
      std::cout << std::endl;

      size_t min_rank(std::numeric_limits<size_t>::max());
      for(auto c: referencers) {
        min_rank = std::min(min_rank, cell_info_map[c].rank);
      } // for

      std::cout << "vertex " << i << " belongs to rank " <<
        min_rank << std::endl;
    } // for
  } // if
} // TEST

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
