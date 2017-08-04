/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_coloring_dcrs_utils_h
#define flecsi_coloring_dcrs_utils_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 24, 2016
//----------------------------------------------------------------------------//

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include "flecsi/coloring/crs.h"
#include "flecsi/topology/closure_utils.h"
#include "flecsi/topology/mesh_definition.h"

namespace flecsi {
namespace coloring {

clog_register_tag(dcrs_utils);

//----------------------------------------------------------------------------//
//! Create a naive coloring suitable for calling a distributed-memory
//! coloring tool, e.g., ParMETIS.
//!
//! @tparam DIMENSION      The entity dimension for which to create a naive
//!                        coloring.
//! @tparam MESH_DIMENSION The dimension of the mesh definition.
//----------------------------------------------------------------------------//

template<
  size_t DIMENSION,
  size_t MESH_DIMENSION
>
inline
std::set<size_t>
naive_coloring(
  topology::mesh_definition__<MESH_DIMENSION> & md
)
{
  std::set<size_t> indices;

  {
  clog_tag_guard(dcrs_utils);

	int size;
	int rank;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //--------------------------------------------------------------------------//
  // Create a naive initial distribution of the indices
  //--------------------------------------------------------------------------//

	size_t quot = md.num_entities(DIMENSION)/size;
	size_t rem = md.num_entities(DIMENSION)%size;

  clog_one(info) << "quot: " << quot << " rem: " << rem << std::endl;

  // Each rank gets the average number of indices, with higher ranks
  // getting an additional index for non-zero remainders.
	size_t init_indices = quot + ((rank >= (size - rem)) ? 1 : 0);

  size_t offset(0);
	for(size_t r(0); r<rank; ++r) {
		offset += quot + ((r >= (size - rem)) ? 1 : 0);
	} // for

  clog_one(info) << "offset: " << offset << std::endl;

  for(size_t i(0); i<init_indices; ++i) {
    indices.insert(offset+i);
  clog_one(info) << "inserting: " << offset+i << std::endl;
  } // for
  } // guard

  return indices;
} // naive_coloring

//----------------------------------------------------------------------------//
//! Create distributed CRS representation of the graph defined by entities
//! of FROM_DIMENSION to TO_DIMENSION through THRU_DIMENSION. The return
//! object will be populated with a naive partitioning suitable for use
//! with coloring tools, e.g., ParMETIS.
//!
//! @tparam FROM_DIMENSION The topological dimension of the entity for which
//!                        the partitioning is requested.
//! @tparam TO_DIMENSION   The topological dimension to search for neighbors.
//! @tparam THRU_DIMENSION The topological dimension through which the neighbor
//!                        connection exists.
//!
//! @param md The mesh definition.
//!
//! @ingroup coloring
//----------------------------------------------------------------------------//

template< 
  std::size_t DIMENSION,
  std::size_t FROM_DIMENSION=DIMENSION,
  std::size_t TO_DIMENSION=DIMENSION,
  std::size_t THRU_DIMENSION = DIMENSION-1
>
inline
dcrs_t
make_dcrs(
  const typename topology::mesh_definition__<DIMENSION> & md
)
{
	int size;
	int rank;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //--------------------------------------------------------------------------//
  // Create a naive initial distribution of the indices
  //--------------------------------------------------------------------------//

	size_t quot = md.num_entities(FROM_DIMENSION)/size;
	size_t rem = md.num_entities(FROM_DIMENSION)%size;

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
    auto neighbors = topology::entity_neighbors<
      FROM_DIMENSION,
      TO_DIMENSION,
      THRU_DIMENSION
    >
    (
      md, 
      dcrs.distribution[rank] + i
    );

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

  return dcrs;
} // make_dcrs

} // namespace coloring
} // namespace flecsi

#endif // flecsi_coloring_dcrs_utils_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
