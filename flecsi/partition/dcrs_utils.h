/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_dmp_dcrs_utils_h
#define flecsi_dmp_dcrs_utils_h

#include <mpi.h>

#include "flecsi/io/definition_utils.h"
#include "flecsi/io/mesh_definition.h"
#include "flecsi/partition/dcrs.h"

///
// \file dcrs_utils.h
// \authors bergen
// \date Initial file creation: Nov 24, 2016
///

namespace flecsi {
namespace dmp {

///
// \param md The mesh definition.
///
dcrs_t
make_dcrs(
  io::mesh_definition_t & md
)
{
	int size;
	int rank;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //--------------------------------------------------------------------------//
  // Create a naive initial distribution of the indices
  //--------------------------------------------------------------------------//

	size_t quot = md.num_cells()/size;
	size_t rem = md.num_cells()%size;

  // Each rank gets the average number of indices, with higher ranks
  // getting an additional index for non-zero remainders.
	size_t init_indices = quot + ((rank >= (size - rem)) ? 1 : 0);

  // Start to initialize the return object.
	dcrs_t dcrs;
	dcrs.distribution.push_back(0);

  // Set the distributions for each rank. This happens on all ranks.
	for(size_t r(0); r<size; ++r) {
		const size_t indices = quot + ((r >= (size - rem)) ? 1 : 0);
		dcrs.distribution.push_back(dcrs.distribution[r] + indices);
	} // for

#if 0
	if(rank == 0) {
		std::cout << "distribution: ";
		for(auto i: dcrs.distribution) {
      std::cout << i << " ";
		} // for
    std::cout << std::endl;
	} // if
#endif

  //--------------------------------------------------------------------------//
  // Create the cell-to-cell graph.
  //--------------------------------------------------------------------------//

  // Set the first offset (always zero).
  dcrs.offsets.push_back(0);

  // Add the graph adjacencies by getting the neighbors of each
  // cell index
  for(size_t i(0); i<init_indices; ++i) {

    // Get the neighboring cells of the current cell index (i) using
    // a matching criteria of "md.dimension()" vertices. The dimension
    // argument will pick neighbors that are adjacent across facets, e.g.,
    // across edges in two dimension, or across faces in three dimensions.
    auto neighbors =
      io::cell_neighbors(md, dcrs.distribution[rank] + i, md.dimension());

#if 0
      if(rank == 1) {
        std::cout << "neighbors: ";
        for(auto i: neighbors) {
          std::cout << i << " ";
        } // for
        std::cout << std::endl;
      } // if
#endif

      for(auto n: neighbors) {
        dcrs.indices.push_back(n);
      } // for

      dcrs.offsets.push_back(dcrs.offsets[i] + neighbors.size());
  } // for

#if 0
  if(rank == 1) {
    std::cout << "offsets: ";
    for(auto i: dcrs.offsets) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;

    std::cout << "indices: ";
    for(auto i: dcrs.indices) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;
  } // if
#endif

  return dcrs;
} // make_dcrs

} // namespace dmp
} // namespace flecsi

#endif // flecsi_dmp_dcrs_utils_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
