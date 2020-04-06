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
#pragma once

/*! @file */

#include <flecsi-config.h>

#include <flecsi/topology/unstructured/closure_utils.hh>
#include <flecsi/topology/unstructured/coloring_types.hh>
#include <flecsi/topology/unstructured/definition.hh>
#include <flecsi/topology/unstructured/index_coloring.hh>
#include <flecsi/topology/unstructured/parallel_definition.hh>
#include <flecsi/utils/crs.hh>
#include <flecsi/utils/mpi_type_traits.hh>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <map>

namespace flecsi {

inline flog::devel_tag dcrs_utils_tag("dcrs_utils");

namespace topology {
namespace unstructured_impl {

/*!
 Create a naive coloring suitable for calling a distributed-memory
 coloring tool, e.g., ParMETIS.

 @tparam DIMENSION      The entity dimension for which to create a naive
                        coloring.
 @tparam MESH_DIMENSION The dimension of the mesh definition.
 */

template<size_t DIMENSION, size_t MESH_DIMENSION>
inline std::set<size_t>
naive_coloring(definition<MESH_DIMENSION> & md) {
  std::set<size_t> indices;

  {
    flog::devel_guard guard(dcrs_utils_tag);

    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //------------------------------------------------------------------------//
    // Create a naive initial distribution of the indices
    //------------------------------------------------------------------------//

    size_t quot = md.num_entities(DIMENSION) / size;
    size_t rem = md.num_entities(DIMENSION) % size;

    flog(info) << "quot: " << quot << " rem: " << rem << std::endl;

    // Each rank gets the average number of indices, with higher ranks
    // getting an additional index for non-zero remainders.
    size_t init_indices = quot + ((size_t(rank) >= (size - rem)) ? 1 : 0);

    size_t offset(0);
    for(int r(0); r < rank; ++r) {
      offset += quot + ((size_t(r) >= (size - rem)) ? 1 : 0);
    } // for

    flog(info) << "offset: " << offset << std::endl;

    for(size_t i(0); i < init_indices; ++i) {
      indices.insert(offset + i);
      flog(info) << "inserting: " << offset + i << std::endl;
    } // for
  } // guard

  return indices;
} // naive_coloring

/*!
  Create distributed CRS representation of the graph defined by entities
  of FROM_DIMENSION to TO_DIMENSION through THRU_DIMENSION. The return
  object will be populated with a naive partitioning suitable for use
  with coloring tools, e.g., ParMETIS.

  @tparam FROM_DIMENSION The topological dimension of the entity for which
                         the partitioning is requested.
  @tparam TO_DIMENSION   The topological dimension to search for neighbors.
  @tparam THRU_DIMENSION The topological dimension through which the neighbor
                         connection exists.

  @param md The mesh definition.

  @ingroup coloring
 */

template<std::size_t DIMENSION,
  std::size_t FROM_DIMENSION = DIMENSION,
  std::size_t TO_DIMENSION = DIMENSION,
  std::size_t THRU_DIMENSION = DIMENSION - 1>
inline flecsi::utils::dcrs
make_dcrs(const definition<DIMENSION> & md) {
  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //--------------------------------------------------------------------------//
  // Create a naive initial distribution of the indices
  //--------------------------------------------------------------------------//

  size_t quot = md.num_entities(FROM_DIMENSION) / size;
  size_t rem = md.num_entities(FROM_DIMENSION) % size;

  // Each rank gets the average number of indices, with higher ranks
  // getting an additional index for non-zero remainders.
  size_t init_indices = quot + ((size_t(rank) >= (size - rem)) ? 1 : 0);

  // Start to initialize the return object.
  flecsi::utils::dcrs dcrs;
  dcrs.distribution.push_back(0);

  // Set the distributions for each rank. This happens on all ranks.
  for(int r(0); r < size; ++r) {
    const size_t indices = quot + ((size_t(r) >= (size - rem)) ? 1 : 0);
    dcrs.distribution.push_back(dcrs.distribution[r] + indices);
  } // for

  //--------------------------------------------------------------------------//
  // Create the cell-to-cell graph.
  //--------------------------------------------------------------------------//

  // Set the first offset (always zero).
  dcrs.offsets.push_back(0);

  // Add the graph adjacencies by getting the neighbors of each
  // cell index
  using cellid = size_t;
  using vertexid = size_t;

  // essentially vertex to cell and cell to cell through vertices
  // connectivity.
  std::map<vertexid, std::vector<cellid>> vertex2cells;
  std::map<cellid, std::vector<cellid>> cell2cells;

  // build global cell to cell through # of shared vertices connectivity
  for(size_t cell(0); cell < md.num_entities(FROM_DIMENSION); ++cell) {
    std::map<cellid, size_t> cell_counts;

    for(auto vertex : md.entities(FROM_DIMENSION, 0, cell)) {
      // build vertex to cell connectivity, O(n_cells * n_polygon_sides)
      // of insert().
      vertex2cells[vertex].push_back(cell);

      // Count the number of times this cell shares a common vertex with
      // some other cell. O(n_cells * n_polygon_sides * vertex to cells degrees)
      for(auto other : vertex2cells[vertex]) {
        if(other != cell)
          cell_counts[other] += 1;
      }
    }

    for(auto count : cell_counts) {
      if(count.second > THRU_DIMENSION) {
        // append cell to cell through "dimension" connectivity, we need to
        // add both directions
        cell2cells[cell].push_back(count.first);
        cell2cells[count.first].push_back(cell);
      }
    }
  }

  // turn subset of cell 2 cell connectivity to dcrs
  for(size_t i(0); i < init_indices; ++i) {
    auto cell = dcrs.distribution[rank] + i;

    for(auto n : cell2cells[cell]) {
      dcrs.indices.push_back(n);
    } // for

    dcrs.offsets.push_back(dcrs.offsets[i] + cell2cells[cell].size());
  }

  return dcrs;
} // make_dcrs

////////////////////////////////////////////////////////////////////////////////
/// \brief Create distributed CRS representation of the graph
////////////////////////////////////////////////////////////////////////////////
template<std::size_t DIMENSION,
  std::size_t FROM_DIMENSION = DIMENSION,
  std::size_t TO_DIMENSION = DIMENSION,
  std::size_t THRU_DIMENSION = DIMENSION - 1>
void
make_dcrs_have_connectivity(const definition<DIMENSION> & md,
  flecsi::utils::dcrs & dcrs) {
  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //--------------------------------------------------------------------------//
  // Create a naive initial distribution of the indices
  //--------------------------------------------------------------------------//

  size_t quot = md.num_entities(FROM_DIMENSION) / size;
  size_t rem = md.num_entities(FROM_DIMENSION) % size;

  // Each rank gets the average number of indices, with higher ranks
  // getting an additional index for non-zero remainders.
  size_t init_indices = quot + ((size_t(rank) >= (size - rem)) ? 1 : 0);

  // Start to initialize the return object.
  dcrs.distribution.push_back(0);

  // Set the distributions for each rank. This happens on all ranks.
  for(int r(0); r < size; ++r) {
    const size_t indices = quot + ((size_t(r) >= (size - rem)) ? 1 : 0);
    dcrs.distribution.push_back(dcrs.distribution[r] + indices);
  } // for

  //--------------------------------------------------------------------------//
  // Create the cell-to-cell graph.
  //--------------------------------------------------------------------------//

  // Set the first offset (always zero).
  dcrs.offsets.push_back(0);

  // Add the graph adjacencies by getting the neighbors of each
  // cell index
  using cellid = size_t;

  // essentially vertex to cell and cell to cell through vertices
  // connectivity.
  std::map<cellid, std::vector<cellid>> cell2other;

  // Travel from the FROM_DIMENSION (cell) to the TO_DIMENSION (other)
  // via the THRU_DIMENSION (other)
  for(size_t cell(dcrs.distribution[rank]); cell < dcrs.distribution[rank + 1];
      ++cell) {
    auto & this_cell2other = cell2other[cell];

    for(auto vertex : md.entities(FROM_DIMENSION, THRU_DIMENSION, cell)) {

      for(auto other : md.entities(THRU_DIMENSION, TO_DIMENSION, vertex)) {

        if(FROM_DIMENSION == TO_DIMENSION) {
          if(cell != other)
            this_cell2other.push_back(other);
        }
        else {
          this_cell2other.push_back(other);
        }
      }
    }
  }

  // turn subset of cell 2 cell connectivity to dcrs
  for(size_t i(0); i < init_indices; ++i) {
    auto cell = dcrs.distribution[rank] + i;
    auto & this_cell2other = cell2other.at(cell);

    std::sort(this_cell2other.begin(), this_cell2other.end());
    auto last = std::unique(this_cell2other.begin(), this_cell2other.end());
    auto first = this_cell2other.begin();
    auto size = std::distance(first, last);

    for(auto it = first; it != last; ++it) {
      dcrs.indices.push_back(*it);
    } // for

    dcrs.offsets.push_back(dcrs.offsets[i] + size);
  }

} // make_dcrs

template<typename SEND_TYPE, typename ID_TYPE, typename RECV_TYPE>
auto
alltoallv(const SEND_TYPE & sendbuf,
  const ID_TYPE & sendcounts,
  const ID_TYPE & senddispls,
  RECV_TYPE & recvbuf,
  const ID_TYPE & recvcounts,
  const ID_TYPE & recvdispls,
  decltype(MPI_COMM_WORLD) comm) {

  const auto mpi_send_t =
    utils::mpi_typetraits<typename SEND_TYPE::value_type>::type();
  const auto mpi_recv_t =
    utils::mpi_typetraits<typename RECV_TYPE::value_type>::type();

  auto num_ranks = sendcounts.size();

  // create storage for the requests
  std::vector<MPI_Request> requests;
  requests.reserve(2 * num_ranks);

  // post receives
  auto tag = 0;

  for(size_t rank = 0; rank < num_ranks; ++rank) {
    auto count = recvcounts[rank];
    if(count > 0) {
      auto buf = recvbuf.data() + recvdispls[rank];
      requests.resize(requests.size() + 1);
      auto & my_request = requests.back();
      auto ret =
        MPI_Irecv(buf, count, mpi_recv_t, rank, tag, comm, &my_request);
      if(ret != MPI_SUCCESS)
        return ret;
    }
  }

  // send the data
  for(size_t rank = 0; rank < num_ranks; ++rank) {
    auto count = sendcounts[rank];
    if(count > 0) {
      auto buf = sendbuf.data() + senddispls[rank];
      requests.resize(requests.size() + 1);
      auto & my_request = requests.back();
      auto ret =
        MPI_Isend(buf, count, mpi_send_t, rank, tag, comm, &my_request);
      if(ret != MPI_SUCCESS)
        return ret;
    }
  }

  // wait for everything to complete
  std::vector<MPI_Status> status(requests.size());
  auto ret = MPI_Waitall(requests.size(), requests.data(), status.data());

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Simple utility for determining which rank owns an id
////////////////////////////////////////////////////////////////////////////////
template<typename T, typename Vector>
T
rank_owner(const Vector & distribution, T i) {
  auto it = std::upper_bound(distribution.begin(), distribution.end(), i);
  flog_assert(it != distribution.end(), "invalid iterator");
  return std::distance(distribution.begin(), it) - 1;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief A simple utility for subdividing an index space into several parts
////////////////////////////////////////////////////////////////////////////////
template<typename Vector>
void
subdivide(size_t nelem, size_t npart, Vector & dist) {

  size_t quot = nelem / npart;
  size_t rem = nelem % npart;

  dist.reserve(npart + 1);
  dist.push_back(0);

  // Set the distributions for each rank. This happens on all ranks.
  // Each rank gets the average number of indices, with higher ranks
  // getting an additional index for non-zero remainders.
  for(size_t r(0); r < npart; ++r) {
    const size_t indices = quot + ((r >= (npart - rem)) ? 1 : 0);
    dist.push_back(dist[r] + indices);
  } // for
};

////////////////////////////////////////////////////////////////////////////////
/// \brief Create distributed CRS representation of the graph
////////////////////////////////////////////////////////////////////////////////
template<std::size_t MESH_DIMENSION>
void
make_dcrs_distributed(const parallel_definition<MESH_DIMENSION> & md,
  size_t from_dimension,
  size_t to_dimension,
  size_t min_connections,
  flecsi::utils::dcrs & dcrs) {
  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // the mpi data type for size_t
  const auto mpi_size_t = utils::mpi_typetraits<size_t>::type();

  //----------------------------------------------------------------------------
  // Get cell partitioning
  //
  // Note: We create a global index space, but this may be different
  // from the global numbering of the original mesh.
  //----------------------------------------------------------------------------

  auto num_cells = md.num_entities(from_dimension);

  // recompute the partioning
  dcrs.clear();
  dcrs.distribution.resize(size + 1);
  dcrs.distribution[0] = 0;

  MPI_Allgather(&num_cells,
    1,
    mpi_size_t,
    dcrs.distribution.data() + 1,
    1,
    mpi_size_t,
    MPI_COMM_WORLD);

  for(int i = 0; i < size; ++i)
    dcrs.distribution[i + 1] += dcrs.distribution[i];

  // starting and ending cell indices
  auto cells_start = dcrs.distribution[rank];

  //----------------------------------------------------------------------------
  // Create a naive initial distribution of the vertices
  //----------------------------------------------------------------------------

  // first figure out the maximum global id on this rank
  const auto & vertex_local_to_global = md.local_to_global(to_dimension);

  size_t max_global_vert_id{0};
  for(auto v : vertex_local_to_global)
    max_global_vert_id = std::max(max_global_vert_id, v);

  // now the global max id
  size_t tot_verts{0};
  MPI_Allreduce(
    &max_global_vert_id, &tot_verts, 1, mpi_size_t, MPI_MAX, MPI_COMM_WORLD);
  tot_verts++;

  decltype(dcrs.distribution) vert_dist;
  subdivide(tot_verts, size, vert_dist);

  //----------------------------------------------------------------------------
  // Create the Local vertex-to-cell graph.
  //----------------------------------------------------------------------------

  // essentially vertex to cell connectivity
  std::map<size_t, std::vector<size_t>> vertex2cell;
  const auto & cells2vertex = md.entities_crs(from_dimension, to_dimension);

  // Travel from the FROM_DIMENSION (cell) to the TO_DIMENSION (other)
  for(size_t ic = 0; ic < num_cells; ++ic) {
    auto cell = cells_start + ic;
    for(auto i = cells2vertex.offsets[ic]; i < cells2vertex.offsets[ic + 1];
        ++i) {
      auto iv = cells2vertex.indices[i];
      auto vertex = vertex_local_to_global[iv];
      vertex2cell[vertex].emplace_back(cell);
    }
  }

  // remove duplicates
  auto remove_duplicates = [](auto & vertex2cell) {
    for(auto & vertex_cells : vertex2cell) {
      auto v = vertex_cells.first;
      auto & cells = vertex_cells.second;
      std::sort(cells.begin(), cells.end());
      auto first = cells.begin();
      auto last = std::unique(first, cells.end());
      cells.erase(last, cells.end());
    }
  };

  remove_duplicates(vertex2cell);

  //----------------------------------------------------------------------------
  // Send vertex information to the deemed vertex-owner
  //----------------------------------------------------------------------------

  // We send the results for each vertex to their owner rank, which was
  // defined using the vertex subdivision above
  std::vector<size_t> sendcounts(size, 0);

  // count
  for(const auto & vs_pair : vertex2cell) {
    size_t global_id = vs_pair.first;
    auto r = rank_owner(vert_dist, global_id);
    // we will be sending vertex id, number of cells, plus cell ids
    if(r != size_t(rank)) {
      auto n = 2 + vs_pair.second.size();
      sendcounts[r] += n;
    }
  }

  // finish displacements
  std::vector<size_t> senddispls(size + 1);
  senddispls[0] = 0;
  for(int r = 0; r < size; ++r) {
    senddispls[r + 1] = senddispls[r] + sendcounts[r];
    sendcounts[r] = 0;
  }

  // fill buffers
  std::vector<size_t> sendbuf(senddispls[size]);

  for(const auto & vs_pair : vertex2cell) {
    size_t global_id = vs_pair.first;
    auto r = rank_owner(vert_dist, global_id);
    // we will be sending vertex id, number of cells, plus cell ids
    if(r != size_t(rank)) {
      // get offset
      auto offset = senddispls[r] + sendcounts[r];
      // populate data
      sendbuf[offset++] = global_id;
      sendbuf[offset++] = vs_pair.second.size();
      for(auto v : vs_pair.second)
        sendbuf[offset++] = v;
      // bump counters
      auto n = 2 + vs_pair.second.size();
      sendcounts[r] += n;
    }
  }

  std::vector<size_t> recvcounts(size, 0);
  auto ret = MPI_Alltoall(sendcounts.data(),
    1,
    mpi_size_t,
    recvcounts.data(),
    1,
    mpi_size_t,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  std::vector<size_t> recvdispls(size + 1);
  recvdispls[0] = 0;
  for(int r = 0; r < size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];
  std::vector<size_t> recvbuf(recvdispls[size]);

  // now send the actual vertex info
  ret = alltoallv(sendbuf,
    sendcounts,
    senddispls,
    recvbuf,
    recvcounts,
    recvdispls,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  //----------------------------------------------------------------------------
  // Append received vertex information to the local vertex-to-cell graph
  //----------------------------------------------------------------------------

  // and add the results to our local list
  std::map<size_t, std::vector<size_t>> vertex2rank;

  for(int r = 0; r < size; ++r) {
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1];) {
      // get vertex
      auto vertex = recvbuf[i];
      ++i;
      // keep track of ranks that share this vertex
      vertex2rank[vertex].emplace_back(r);
      flog_assert(i < recvdispls[r + 1], "invalid index");
      // unpack cell neighbors
      auto n = recvbuf[i];
      ++i;
      for(size_t j = 0; j < n; ++j) {
        flog_assert(i < recvdispls[r + 1], "invalid index");
        auto cell = recvbuf[i];
        ++i;
        // might not already be there
        vertex2cell[vertex].emplace_back(cell);
      }
    }
  }

  // remove duplicates
  remove_duplicates(vertex2cell);

  //----------------------------------------------------------------------------
  // Send back the final results for shared vertices
  //----------------------------------------------------------------------------

  // now perpare to send results back

  // count send buffer size
  std::fill(sendcounts.begin(), sendcounts.end(), 0);
  for(const auto & vertex_pair : vertex2rank) {
    auto vertex = vertex_pair.first;
    for(auto r : vertex_pair.second) {
      if(r != size_t(rank)) { // should always enter anyway!
        // better already be there
        const auto & cells = vertex2cell.at(vertex);
        // we will be sending vertex id, number of cells, plus cell ids
        auto n = 2 + cells.size();
        sendcounts[r] += n;
      }
    }
  }

  // finish displacements
  senddispls[0] = 0;
  for(int r = 0; r < size; ++r)
    senddispls[r + 1] = senddispls[r] + sendcounts[r];

  // resize send buffer
  sendbuf.clear();
  sendbuf.resize(senddispls[size]);

  // now fill buffer
  std::fill(sendcounts.begin(), sendcounts.end(), 0);
  for(const auto & vertex_pair : vertex2rank) {
    auto vertex = vertex_pair.first;
    for(auto r : vertex_pair.second) {
      if(r != size_t(rank)) { // should always enter anyway!
        auto j = senddispls[r] + sendcounts[r];
        // better already be there
        const auto & cells = vertex2cell.at(vertex);
        // we will be sending vertex id, number of cells, plus cell ids
        sendbuf[j++] = vertex;
        sendbuf[j++] = cells.size();
        for(auto c : cells)
          sendbuf[j++] = c;
        // increment send counter
        sendcounts[r] = j - senddispls[r];
      }
    }
  }

  // send counts
  std::fill(recvcounts.begin(), recvcounts.end(), 0);
  ret = MPI_Alltoall(sendcounts.data(),
    1,
    mpi_size_t,
    recvcounts.data(),
    1,
    mpi_size_t,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating back vertex counts");

  // how much info will we be receiving
  recvdispls[0] = 0;
  for(int r = 0; r < size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];

  // how much info will we be receiving
  recvbuf.clear();
  recvbuf.resize(recvdispls.at(size));

  // now send the final vertex info back
  ret = alltoallv(sendbuf,
    sendcounts,
    senddispls,
    recvbuf,
    recvcounts,
    recvdispls,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating new vertices");

  //----------------------------------------------------------------------------
  // Append received vertex information to the local vertex-to-cell graph
  //----------------------------------------------------------------------------

  for(int r = 0; r < size; ++r) {
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1];) {
      // get vertex
      auto vertex = recvbuf[i];
      ++i;

      // keep track of ranks that share this vertex
      vertex2rank[vertex].emplace_back(r);
      flog_assert(i < recvdispls[r + 1], "invalid index");

      // unpack cell neighbors
      auto n = recvbuf[i];
      ++i;
      for(size_t j = 0; j < n; ++j) {
        flog_assert(i < recvdispls[r + 1], "invalid index");
        auto cell = recvbuf[i];
        ++i;
        // might not already be there
        vertex2cell[vertex].emplace_back(cell);
      }
    }
  }

  // remove duplicates
  remove_duplicates(vertex2cell);

  //----------------------------------------------------------------------------
  // Invert for final cell-to-cell graph
  //----------------------------------------------------------------------------

  // Set the first offset (always zero).
  dcrs.offsets.reserve(num_cells);
  dcrs.indices.reserve(from_dimension * from_dimension * num_cells); // guess
  dcrs.offsets.push_back(0);

  std::map<size_t, size_t> cell_counts;

  for(size_t ic = 0; ic < num_cells; ++ic) {
    auto cell = cells_start + ic;

    cell_counts.clear();

    // iterate over vertices
    auto start = cells2vertex.offsets[ic];
    auto end = cells2vertex.offsets[ic + 1];
    for(auto i = start; i < end; ++i) {
      auto iv = cells2vertex.indices[i];
      auto vertex = vertex_local_to_global[iv];
      // now count attached cells
      for(auto other : vertex2cell.at(vertex)) {
        // new entries are initialized to zero
        if(other != cell)
          cell_counts[other] += 1;
      }
    }

    // now add results
    int num_connections{0};

    for(auto count : cell_counts) {
      if(count.second >= min_connections) {
        dcrs.indices.emplace_back(count.first);
        num_connections++;
      }
    }

    dcrs.offsets.emplace_back(dcrs.offsets.back() + num_connections);
  }

} // make_dcrs

////////////////////////////////////////////////////////////////////////////////
/// \brief Migrate pieces of a mesh from one processor to another
////////////////////////////////////////////////////////////////////////////////
template<std::size_t DIMENSION>
void
migrate(size_t dimension,
  const std::vector<size_t> & partitioning,
  flecsi::utils::dcrs & dcrs,
  parallel_definition<DIMENSION> & md) {

  int comm_size, comm_rank;

  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  //----------------------------------------------------------------------------
  // Pack information together.
  //
  // We need to query the mesh definition to see if there is extra info we
  // want to send.
  //----------------------------------------------------------------------------

  using byte_t = typename parallel_definition<DIMENSION>::byte_t;

  std::vector<size_t> sendcounts(comm_size, 0);
  std::vector<size_t> senddispls(comm_size + 1);
  std::vector<byte_t> sendbuf;
  std::vector<size_t> erase_local_ids;

  senddispls[0] = 0;
  auto num_elements = dcrs.size();

  // Find the indices we need torequest.
  for(int rank(0); rank < comm_size; ++rank) {

    // keep track of the buffer beginning
    auto start_buf = sendbuf.size();

    // loop over entities we partitionied
    for(size_t local_id(0); local_id < num_elements && rank != comm_rank;
        ++local_id) {

      // the global id
      auto global_id = dcrs.distribution[comm_rank] + local_id;

      //------------------------------------------------------------------------
      // No migration necessary
      if(partitioning[local_id] == size_t(comm_rank)) {
        // just keep track of the global ids
      }
      //------------------------------------------------------------------------
      // Entity to be migrated
      else if(partitioning[local_id] == size_t(rank)) {
        // mark for deletion
        erase_local_ids.emplace_back(local_id);
        // global id
        cast_insert(&global_id, 1, sendbuf);
        // dcrs info
        auto start = dcrs.offsets[local_id];
        auto num_offsets = dcrs.offsets[local_id + 1] - start;
        cast_insert(&num_offsets, 1, sendbuf);
        cast_insert(&dcrs.indices[start], num_offsets, sendbuf);
        // now specific info related to mesh
        md.pack(dimension, local_id, sendbuf);
      }
    } // for entities

    // keep track of offsets
    sendcounts[rank] = sendbuf.size() - start_buf;
    senddispls[rank + 1] = senddispls[rank] + sendcounts[rank];

  } // for ranks

  //----------------------------------------------------------------------------
  // Erase entities to be migrated
  //----------------------------------------------------------------------------

  if(erase_local_ids.size()) {

    // sort them first
    std::sort(erase_local_ids.begin(), erase_local_ids.end());
    auto last = std::unique(erase_local_ids.begin(), erase_local_ids.end());
    flog_assert(last == erase_local_ids.end(), "duplicate ids to delete");

    // erase dcrs elements
    dcrs.erase(erase_local_ids);

    // erase mesh elements
    md.erase(dimension, erase_local_ids);

    // decrement counter
    num_elements -= erase_local_ids.size();
  }

  //----------------------------------------------------------------------------
  // Send information.
  //----------------------------------------------------------------------------

  // the mpi data type for size_t
  const auto mpi_size_t = utils::mpi_typetraits<size_t>::type();

  std::vector<size_t> recvcounts(comm_size, 0);

  auto ret = MPI_Alltoall(sendcounts.data(),
    1,
    mpi_size_t,
    recvcounts.data(),
    1,
    mpi_size_t,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  std::vector<size_t> recvdispls(comm_size + 1);
  recvdispls[0] = 0;
  for(int r = 0; r < comm_size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];
  std::vector<byte_t> recvbuf(recvdispls[comm_size]);

  // now send the actual info
  ret = alltoallv(sendbuf,
    sendcounts,
    senddispls,
    recvbuf,
    recvcounts,
    recvdispls,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  //----------------------------------------------------------------------------
  // Unpack information.
  //----------------------------------------------------------------------------

  // Add indices to primary
  for(int rank(0); rank < comm_size; ++rank) {

    auto start = recvdispls[rank];
    auto end = recvdispls[rank + 1];
    const auto * buffer = &recvbuf[start];
    const auto * buffer_end = &recvbuf[end];

    // Typically, one would check for equality between an iterator address
    // and the ending pointer address.  This might cause an infinate loop
    // if there is an error in unpacking.  So I think testing on byte
    // index is safer since there is no danger of iterating past the end.
    for(size_t i = start; i < end;) {

      // capture start of buffer
      const auto * buffer_start = buffer;

      // the local_id
      auto local_id = num_elements++;

      // global id
      size_t global_id;
      uncast(buffer, 1, &global_id);

      // dcrs info
      auto last = dcrs.offsets[local_id];

      size_t num_offsets;
      uncast(buffer, 1, &num_offsets);
      dcrs.offsets.emplace_back(last + num_offsets);

      dcrs.indices.resize(dcrs.indices.size() + num_offsets);
      uncast(buffer, num_offsets, &dcrs.indices[last]);

      // now specific info related to mesh
      md.unpack(dimension, local_id, buffer);

      // increment pointer
      i += std::distance(buffer_start, buffer);

    } // for

    // make sre we are at the right spot in the buffer
    flog_assert(buffer == buffer_end, "Unpacking mismatch");

  } // for

  flog_assert(num_elements == dcrs.size(),
    "Mismatch in the final number of elements after migration");

  flog_assert(num_elements > 0,
    "At least one rank has an empty primary coloring. Please either "
    "increase the problem size or use fewer ranks");

  // recompute the partioning
  dcrs.distribution.clear();
  dcrs.distribution.resize(comm_size + 1);
  dcrs.distribution[0] = 0;

  MPI_Allgather(&num_elements,
    1,
    mpi_size_t,
    dcrs.distribution.data() + 1,
    1,
    mpi_size_t,
    MPI_COMM_WORLD);

  for(int i = 0; i < comm_size; ++i)
    dcrs.distribution[i + 1] += dcrs.distribution[i];
}

inline void
get_owner_info(const flecsi::utils::dcrs & dcrs,
  index_coloring & entities,
  coloring_info & color_info) {

  int comm_size, comm_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  //----------------------------------------------------------------------------
  // Determine primary/ghost division
  //----------------------------------------------------------------------------

  // starting and ending cell indices
  auto start = dcrs.distribution[comm_rank];
  auto end = dcrs.distribution[comm_rank + 1] - 1;
  auto num_entities = end - start + 1;
  // size_t num_ghost = 0;

  // keep track of shared ranks
  std::set<size_t> shared_ranks;

  for(size_t local_id = 0; local_id < num_entities; ++local_id) {

    // determine clobal id
    auto global_id = start + local_id;
    // reset storage for shared rank ids
    shared_ranks.clear();

    for(auto i = dcrs.offsets[local_id]; i < dcrs.offsets[local_id + 1]; ++i) {
      // who owns the neighbor cell
      auto neighbor = dcrs.indices[i];
      auto rank = rank_owner(dcrs.distribution, neighbor);

      // neighbor is a ghost cell
      if(rank != size_t(comm_rank)) {
        // create possible new ghost
        // auto ghost_id = num_entities+num_ghost;
        auto ghost_id = neighbor - dcrs.distribution[rank];
        auto e = entity_info(neighbor, rank, ghost_id, comm_rank);
        // search for it
        auto it = entities.ghost.find(e);
        // if it has not been added, then add it!
        if(it == entities.ghost.end()) {
          entities.ghost.emplace(e);
          color_info.ghost_owners.insert(rank);
          // num_ghost++;
        }
        // keep track of shared ranks for the parent cell
        shared_ranks.emplace(rank);
      }
    } // neighbors

    // the original cell is exclusive
    if(shared_ranks.empty()) {
      entities.exclusive.emplace(entity_info(global_id, comm_rank, local_id));
    }
    // otherwise it must be shared
    else {
      entities.shared.emplace(
        entity_info(global_id, comm_rank, local_id, shared_ranks));
      color_info.shared_users.insert(shared_ranks.begin(), shared_ranks.end());
    }
  }

  // store the sizes of each set
  color_info.exclusive = entities.exclusive.size();
  color_info.shared = entities.shared.size();
  color_info.ghost = entities.ghost.size();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Color an auxiliary index space like vertices or edges
////////////////////////////////////////////////////////////////////////////////
#if 0 // FIXME: DO WE NEED THIS?
inline void
color_entities(const flecsi::utils::crs & cells2entity,
  const std::vector<size_t> & local2global,
  const std::map<size_t, size_t> & global2local,
  const index_coloring & cells,
  index_coloring & entities,
  coloring_info & color_info,
  size_t & connectivity_counts) {

  int comm_size, comm_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  // the mpi data type for size_t
  const auto mpi_size_t = utils::mpi_typetraits<size_t>::type();

  //----------------------------------------------------------------------------
  // Start marking all vertices as exclusive
  //----------------------------------------------------------------------------

  // marked as exclusive by default
  auto num_ents = local2global.size();
  std::vector<bool> exclusive(num_ents, true);

  // set of possible shared vertices
  std::vector<size_t> potential_shared;

  // now mark shared
  for(const auto & c : cells.shared) {
    auto start = cells2entity.offsets[c.offset];
    auto end = cells2entity.offsets[c.offset + 1];
    for(size_t i = start; i < end; ++i) {
      auto local_id = cells2entity.indices[i];
      exclusive[local_id] = false;
      potential_shared.emplace_back(local_id);
    }
  }

  // remove duplicates
  auto remove_unique = [](auto & list) {
    std::sort(list.begin(), list.end());
    auto last = std::unique(list.begin(), list.end());
    list.erase(last, list.end());
  };
  remove_unique(potential_shared);

  //----------------------------------------------------------------------------
  // Determine cell-to-vertex connecitivity for my shared cells
  //----------------------------------------------------------------------------

  // first count for storage
  std::vector<size_t> sendcounts(comm_size, 0);
  std::vector<size_t> senddispls(comm_size + 1);

  for(const auto & c : cells.shared) {
    // get cell info
    auto start = cells2entity.offsets[c.offset];
    auto end = cells2entity.offsets[c.offset + 1];
    auto n = end - start;
    // loop over shared ranks
    for(auto r : c.shared) {
      // we will be sending the cell number of vertices, plus vertices
      if(r != comm_rank)
        sendcounts[r] += 1 + n;
    }
  }

  // finish displacements
  senddispls[0] = 0;
  for(size_t r = 0; r < comm_size; ++r)
    senddispls[r + 1] = senddispls[r] + sendcounts[r];

  // now fill buffers
  std::vector<size_t> sendbuf(senddispls[comm_size]);
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  for(const auto & c : cells.shared) {
    // get cell info
    auto start = cells2entity.offsets[c.offset];
    auto end = cells2entity.offsets[c.offset + 1];
    auto n = end - start;
    // loop over shared ranks
    for(auto r : c.shared) {
      // we will be sending number of vertices, plus vertices
      if(r != comm_rank) {
        // first the id and size
        auto j = senddispls[r] + sendcounts[r];
        sendbuf[j++] = n;
        for(auto i = start; i < end; ++i) {
          auto local_id = cells2entity.indices[i];
          auto global_id = local2global[local_id];
          sendbuf[j++] = global_id;
        }
        // increment send counter
        sendcounts[r] = j - senddispls[r];
      }
    } // shared ranks
  }

  //----------------------------------------------------------------------------
  // Send shared information
  //----------------------------------------------------------------------------

  // send the counts
  std::vector<size_t> recvcounts(comm_size);
  auto ret = MPI_Alltoall(sendcounts.data(), 1, mpi_size_t, recvcounts.data(),
    1, mpi_size_t, MPI_COMM_WORLD);
  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  std::vector<size_t> recvdispls(comm_size + 1);
  recvdispls[0] = 0;
  for(size_t r = 0; r < comm_size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];
  std::vector<size_t> recvbuf(recvdispls[comm_size]);

  // figure out the size of the connectivity array
  connectivity_counts = cells2entity.indices.size();

  // now send the actual vertex info
  ret = alltoallv(sendbuf, sendcounts, senddispls, recvbuf, recvcounts,
    recvdispls, MPI_COMM_WORLD);
  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  //----------------------------------------------------------------------------
  // Unpack results
  //----------------------------------------------------------------------------

  // set of possible ghost vertices
  std::vector<size_t> potential_ghost;

  for(size_t r = 0; r < comm_size; ++r) {
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1];) {
      // num of vertices
      auto n = recvbuf[i];
      i++;
      connectivity_counts += n;
      // no sift through ghost vertices
      for(auto j = 0; j < n; ++j, ++i) {
        // local and global ids of the vertex (relative to sender)
        auto ent_global_id = recvbuf[i];
        // Don't bother checking if i have this vertex yet, even if I already
        // have it, it might be a ghost
        potential_ghost.emplace_back(ent_global_id);
      } // vertices
    }
  }

  // remove duplicates
  remove_unique(potential_ghost);

  //----------------------------------------------------------------------------
  // Create a naive initial distribution of the vertices
  //----------------------------------------------------------------------------

  // first figure out the maximum global id on this rank
  size_t max_global_ent_id{0};
  for(auto v : local2global)
    max_global_ent_id = std::max(max_global_ent_id, v);

  // now the global max id
  size_t tot_ents{0};
  MPI_Allreduce(
    &max_global_ent_id, &tot_ents, 1, mpi_size_t, MPI_MAX, MPI_COMM_WORLD);
  tot_ents++;

  std::vector<size_t> ent_dist;
  subdivide(tot_ents, comm_size, ent_dist);

  //----------------------------------------------------------------------------
  // Send shared vertex information to the deemed vertex-owner
  //
  // After this, we know who is the sharer of a particular vertex
  //----------------------------------------------------------------------------

  struct owner_t {
    size_t rank;
    size_t local_id;
    std::vector<size_t> ghost_ranks;
    owner_t(size_t r, size_t id) : rank(r), local_id(id) {}
  };
  std::map<size_t, owner_t> entities2rank;

  // We send the results for each vertex to their owner rank, which was
  // defined using the vertex subdivision above
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  for(auto local_id : potential_shared) {
    auto global_id = local2global[local_id];
    // who owns this vertex
    auto rank = rank_owner(ent_dist, global_id);
    // we will be sending global and local ids
    if(rank != comm_rank)
      sendcounts[rank] += 2;
  } // vert

  // finish displacements
  senddispls[0] = 0;
  for(size_t r = 0; r < comm_size; ++r)
    senddispls[r + 1] = senddispls[r] + sendcounts[r];

  // now fill buffers
  sendbuf.clear();
  sendbuf.resize(senddispls[comm_size]);
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  for(const auto & local_id : potential_shared) {
    auto global_id = local2global[local_id];
    // who owns this vertex
    auto rank = rank_owner(ent_dist, global_id);
    // we will be sending vertex id, number of cells, plus cell ids
    if(rank != comm_rank) {
      auto i = senddispls[rank] + sendcounts[rank];
      sendbuf[i] = global_id;
      sendbuf[i + 1] = local_id;
      sendcounts[rank] += 2;
    }
    // if its ours, just add it to the list
    else {
      entities2rank.emplace(global_id, owner_t{rank, local_id});
    }
  } // vert

  // send counts
  ret = MPI_Alltoall(sendcounts.data(), 1, mpi_size_t, recvcounts.data(), 1,
    mpi_size_t, MPI_COMM_WORLD);
  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  recvdispls[0] = 0;
  for(size_t r = 0; r < comm_size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];

  recvbuf.clear();
  recvbuf.resize(recvdispls[comm_size]);

  // now send the actual vertex info
  ret = alltoallv(sendbuf, sendcounts, senddispls, recvbuf, recvcounts,
    recvdispls, MPI_COMM_WORLD);
  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  // upack results
  for(size_t r = 0; r < comm_size; ++r) {
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1]; i += 2) {
      auto global_id = recvbuf[i];
      auto local_id = recvbuf[i + 1];
      auto res = entities2rank.emplace(global_id, owner_t{r, local_id});
      if(!res.second && r < res.first->second.rank) {
        res.first->second = {r, local_id};
      }
    }
  }

  //----------------------------------------------------------------------------
  // Send ghost vertex information to the deemed vertex-owner
  //----------------------------------------------------------------------------

  // We send the results for each vertex to their owner rank, which was
  // defined using the vertex subdivision above
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  for(auto global_id : potential_ghost) {
    // who owns this vertex
    auto rank = rank_owner(ent_dist, global_id);
    // we will be sending global ids
    if(rank != comm_rank)
      sendcounts[rank]++;
  } // vert

  // finish displacements
  senddispls[0] = 0;
  for(size_t r = 0; r < comm_size; ++r)
    senddispls[r + 1] = senddispls[r] + sendcounts[r];

  // now fill buffers
  sendbuf.clear();
  sendbuf.resize(senddispls[comm_size]);
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  for(const auto & global_id : potential_ghost) {
    // who owns this vertex
    auto rank = rank_owner(ent_dist, global_id);
    // we will be sending global ids
    if(rank != comm_rank) {
      auto i = senddispls[rank] + sendcounts[rank];
      sendbuf[i] = global_id;
      sendcounts[rank]++;
    }
    // otherwise, i am responsible for this ghost
    else {
      auto & owner = entities2rank.at(global_id);
      if(owner.rank != rank) {
        owner.ghost_ranks.emplace_back(rank);
      }
    }
  } // vert

  // send counts
  ret = MPI_Alltoall(sendcounts.data(), 1, mpi_size_t, recvcounts.data(), 1,
    mpi_size_t, MPI_COMM_WORLD);
  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  recvdispls[0] = 0;
  for(size_t r = 0; r < comm_size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];

  recvbuf.clear();
  recvbuf.resize(recvdispls[comm_size]);

  // now send the actual vertex info
  ret = alltoallv(sendbuf, sendcounts, senddispls, recvbuf, recvcounts,
    recvdispls, MPI_COMM_WORLD);
  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  // upack results
  for(size_t r = 0; r < comm_size; ++r) {
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1]; i++) {
      auto global_id = recvbuf[i];
      // definately a ghost!
      auto & owner = entities2rank.at(global_id);
      if(owner.rank != r) {
        owner.ghost_ranks.emplace_back(r);
      }
    }
  }

  //----------------------------------------------------------------------------
  // Send back the ghost/shared information to everyone that needs it
  //----------------------------------------------------------------------------
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  // figure out send counts
  std::vector<size_t> ranks;
  for(const auto & pair : entities2rank) {
    const auto owner = pair.second;
    // who am i sending to
    ranks.assign(owner.ghost_ranks.begin(), owner.ghost_ranks.end());
    ranks.push_back(owner.rank);
    // we will be sending id+rank+offset+num_ghost+ghost_ranks
    auto n = 4 + owner.ghost_ranks.size();
    for(auto rank : ranks) {
      if(rank != comm_rank)
        sendcounts[rank] += n;
    }
  } // vert

  // finish displacements
  senddispls[0] = 0;
  for(size_t r = 0; r < comm_size; ++r)
    senddispls[r + 1] = senddispls[r] + sendcounts[r];

  // now fill buffers
  sendbuf.clear();
  sendbuf.resize(senddispls[comm_size]);
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  for(const auto & pair : entities2rank) {
    auto global_id = pair.first;
    const auto owner = pair.second;
    // who am i sending to
    ranks.assign(owner.ghost_ranks.begin(), owner.ghost_ranks.end());
    ranks.push_back(owner.rank);
    // we will be sending id+rank+offset+num_ghost+ghost_ranks
    auto n = 4 + owner.ghost_ranks.size();
    for(auto rank : ranks) {
      // we will be sending global ids
      if(rank != comm_rank) {
        auto i = senddispls[rank] + sendcounts[rank];
        sendbuf[i++] = global_id;
        sendbuf[i++] = owner.rank;
        sendbuf[i++] = owner.local_id;
        sendbuf[i++] = owner.ghost_ranks.size();
        for(auto r : owner.ghost_ranks)
          sendbuf[i++] = r;
        sendcounts[rank] += n;
      }
    }
  } // vert

  // send counts
  ret = MPI_Alltoall(sendcounts.data(), 1, mpi_size_t, recvcounts.data(), 1,
    mpi_size_t, MPI_COMM_WORLD);
  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  recvdispls[0] = 0;
  for(size_t r = 0; r < comm_size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];

  recvbuf.clear();
  recvbuf.resize(recvdispls[comm_size]);

  // now send the actual vertex info
  ret = alltoallv(sendbuf, sendcounts, senddispls, recvbuf, recvcounts,
    recvdispls, MPI_COMM_WORLD);
  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  // upack results
  for(size_t r = 0; r < comm_size; ++r) {
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1];) {
      auto global_id = recvbuf[i];
      i++;
      auto rank = recvbuf[i];
      i++;
      auto local_id = recvbuf[i];
      i++;
      auto num_shared = recvbuf[i];
      i++;
      auto it = entities2rank.emplace(global_id, owner_t{rank, local_id});
      flog_assert(it.second,
             "did not insert vertex, which means multiple owners sent it");
      for(size_t j = 0; j < num_shared; ++j, ++i) {
        it.first->second.ghost_ranks.emplace_back(recvbuf[i]);
      }
    }
  }

  //----------------------------------------------------------------------------
  // Now we can set exclusive, shared and ghost
  //----------------------------------------------------------------------------

  // exclusive
  for(size_t local_id = 0; local_id < num_ents; ++local_id) {
    if(exclusive[local_id]) {
      auto global_id = local2global[local_id];
      entities.exclusive.emplace(global_id, comm_rank, local_id);
    }
  }

  // shared and ghost
  for(const auto pair : entities2rank) {
    auto global_id = pair.first;
    auto owner = pair.second;
    // if i am the owner, shared
    if(owner.rank == comm_rank) {
      flog_assert(owner.ghost_ranks.size(), "shared/ghost has no ghost ranks!");
      auto it = entities.shared.emplace(
        global_id, owner.rank, owner.local_id, owner.ghost_ranks);
      color_info.shared_users.insert(
        it.first->shared.begin(), it.first->shared.end());
    }
    // otherwise this "may" be a ghost
    else {
      auto is_ghost = std::binary_search(
        potential_ghost.begin(), potential_ghost.end(), global_id);
      if(is_ghost) {
        color_info.ghost_owners.insert(owner.rank);
      } // is ghost
    }
  }

  // entitiy summaries
  color_info.exclusive = entities.exclusive.size();
  color_info.shared = entities.shared.size();
  color_info.ghost = entities.ghost.size();
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// \brief Number an index space like edges or faces
////////////////////////////////////////////////////////////////////////////////
template<std::size_t MESH_DIMENSION>
void
match_ids(const parallel_definition<MESH_DIMENSION> & md,
  size_t dimension,
  std::vector<size_t> & local2global,
  std::map<size_t, size_t> & global2local) {

  int comm_size, comm_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  // the mpi data type for size_t
  const auto mpi_size_t = utils::mpi_typetraits<size_t>::type();

  //----------------------------------------------------------------------------
  // Create a naive initial distribution of the vertices
  //----------------------------------------------------------------------------

  // first figure out the maximum global id on this rank
  const auto & vertex_local2global = md.local_to_global(0);

  size_t max_global_vert_id{0};
  for(auto v : vertex_local2global)
    max_global_vert_id = std::max(max_global_vert_id, v);

  // now the global max id
  size_t tot_verts{0};
  MPI_Allreduce(
    &max_global_vert_id, &tot_verts, 1, mpi_size_t, MPI_MAX, MPI_COMM_WORLD);
  tot_verts++;

  std::vector<size_t> vert_dist;
  subdivide(tot_verts, comm_size, vert_dist);

  //----------------------------------------------------------------------------
  // Send each edge to the edge owner
  //
  // The vertex with the lowest id owns the edge
  //----------------------------------------------------------------------------

  const auto & entities2vertex = md.entities_crs(dimension, 0);

  // map the vertices to an id
  std::map<std::vector<size_t>, size_t> entities;

  // storage for vertex sorting
  std::vector<size_t> sorted_vs;

  // a utility function for sorting and adding to the map
  auto add_to_map = [&](const auto & vs, auto & vertices2id, auto id) {
    sorted_vs.assign(vs.begin(), vs.end());
    std::sort(sorted_vs.begin(), sorted_vs.end());
    auto res = vertices2id.emplace(sorted_vs, id);
    return res.first;
  };

  // first count for storage
  std::vector<size_t> sendcounts(comm_size, 0);
  std::vector<size_t> senddispls(comm_size + 1);

  // storage for global ids
  std::vector<size_t> global_vs;

  for(const auto & vs : entities2vertex) {
    // convert to global ids
    global_vs.clear();
    global_vs.reserve(vs.size());
    for(auto v : vs)
      global_vs.emplace_back(vertex_local2global.at(v));
    // get the rank owner
    auto it = std::min_element(global_vs.begin(), global_vs.end());
    auto r = rank_owner(vert_dist, *it);
    // we will be sending the number of vertices, plus the vertices
    if(r != size_t(comm_rank))
      sendcounts[r] += 1 + vs.size();
  }

  // finish displacements
  senddispls[0] = 0;
  for(int r = 0; r < comm_size; ++r)
    senddispls[r + 1] = senddispls[r] + sendcounts[r];

  // now fill buffers
  std::vector<size_t> sendbuf(senddispls[comm_size]);
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  for(const auto & vs : entities2vertex) {
    // convert to global ids
    global_vs.clear();
    global_vs.reserve(vs.size());
    for(auto v : vs)
      global_vs.emplace_back(vertex_local2global.at(v));
    // get the rank owner
    auto it = std::min_element(global_vs.begin(), global_vs.end());
    auto r = rank_owner(vert_dist, *it);
    // we will be sending the number of vertices, plus the vertices
    if(r != size_t(comm_rank)) {
      // first the id and size
      auto j = senddispls[r] + sendcounts[r];
      sendbuf[j++] = vs.size();
      for(auto v : global_vs)
        sendbuf[j++] = v;
      // increment send counter
      sendcounts[r] = j - senddispls[r];
    }
    // otherwise, if its mine, add it
    else {
      add_to_map(global_vs, entities, entities.size());
    }
  }

  //----------------------------------------------------------------------------
  // Send information
  //----------------------------------------------------------------------------

  // send the counts
  std::vector<size_t> recvcounts(comm_size);
  auto ret = MPI_Alltoall(sendcounts.data(),
    1,
    mpi_size_t,
    recvcounts.data(),
    1,
    mpi_size_t,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  std::vector<size_t> recvdispls(comm_size + 1);
  recvdispls[0] = 0;
  for(int r = 0; r < comm_size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];
  std::vector<size_t> recvbuf(recvdispls[comm_size]);

  // now send the actual vertex info
  ret = alltoallv(sendbuf,
    sendcounts,
    senddispls,
    recvbuf,
    recvcounts,
    recvdispls,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  //----------------------------------------------------------------------------
  // Unpack results
  //----------------------------------------------------------------------------

  using pointer_t = std::decay_t<decltype(entities.begin())>;

  // keep track of who needs what
  std::map<size_t, std::vector<pointer_t>> rank_edges;

  for(int r = 0; r < comm_size; ++r) {
    auto & rank_data = rank_edges[r];
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1];) {
      auto n = recvbuf[i];
      auto vs = utils::make_array_ref(&recvbuf[i + 1], n);
      auto it = add_to_map(vs, entities, entities.size());
      rank_data.emplace_back(it);
      i += n + 1;
    }

    // sort and remove unique entries
    std::sort(rank_data.begin(),
      rank_data.end(),
      [](const auto & a, const auto & b) { return a->second < b->second; });
    auto last = std::unique(rank_data.begin(), rank_data.end());
    rank_data.erase(last, rank_data.end());
  }

  //----------------------------------------------------------------------------
  // Come up with a global numbering
  //----------------------------------------------------------------------------

  size_t my_edges{entities.size()};

  std::vector<size_t> edge_dist(comm_size + 1);
  edge_dist[0] = 0;

  MPI_Allgather(
    &my_edges, 1, mpi_size_t, &edge_dist[1], 1, mpi_size_t, MPI_COMM_WORLD);

  for(int r = 0; r < comm_size; ++r)
    edge_dist[r + 1] += edge_dist[r];

  // bump all my local edge counts up
  for(auto & pair : entities)
    pair.second += edge_dist[comm_rank];

  //----------------------------------------------------------------------------
  // Send back the finished edges to those that sent you the pairs
  //----------------------------------------------------------------------------

  // counts for storage
  std::fill(sendcounts.begin(), sendcounts.end(), 0);
  // and fill buffers
  sendbuf.clear();

  for(int r = 0; r < comm_size; ++r) {
    if(r == comm_rank)
      continue;
    // we will be sending the edge id, number of vertices, plus the vertices
    for(auto edge : rank_edges.at(r)) {
      auto global_id = edge->second;
      auto n = edge->first.size();
      sendcounts[r] += 2 + n;
      sendbuf.emplace_back(global_id);
      sendbuf.emplace_back(n);
      for(auto v : edge->first)
        sendbuf.emplace_back(v);
    }
  }

  // finish displacements
  senddispls[0] = 0;
  for(int r = 0; r < comm_size; ++r)
    senddispls[r + 1] = senddispls[r] + sendcounts[r];

  //----------------------------------------------------------------------------
  // Send information
  //----------------------------------------------------------------------------

  // send the counts
  ret = MPI_Alltoall(sendcounts.data(),
    1,
    mpi_size_t,
    recvcounts.data(),
    1,
    mpi_size_t,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  recvdispls[0] = 0;
  for(int r = 0; r < comm_size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];
  recvbuf.clear();
  recvbuf.resize(recvdispls[comm_size]);

  // now send the actual vertex info
  ret = alltoallv(sendbuf,
    sendcounts,
    senddispls,
    recvbuf,
    recvcounts,
    recvdispls,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  //----------------------------------------------------------------------------
  // Unpack results
  //----------------------------------------------------------------------------

  for(int r = 0; r < comm_size; ++r) {
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1];) {
      auto global_id = recvbuf[i];
      auto n = recvbuf[i + 1];
      auto vs = utils::make_array_ref(&recvbuf[i + 2], n);
      i += n + 2;
      add_to_map(vs, entities, global_id);
    }
  }

  //----------------------------------------------------------------------------
  // Figure local ids for the edges
  //----------------------------------------------------------------------------

  for(const auto & vs : entities2vertex) {
    // convert to global ids
    global_vs.clear();
    global_vs.reserve(vs.size());

    for(auto v : vs)
      global_vs.emplace_back(vertex_local2global.at(v));

    // sort the vertices and find the edge
    std::sort(global_vs.begin(), global_vs.end());
    auto it = entities.find(global_vs);
    flog_assert(it != entities.end(), "messed up setting entity ids");

    // figure out the edges global id
    auto global_id = it->second;
    global2local.emplace(global_id, local2global.size());
    local2global.emplace_back(global_id);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Color an auxiliary index space like vertices or edges
////////////////////////////////////////////////////////////////////////////////
inline void
ghost_connectivity(const flecsi::utils::crs & from2to,
  const std::vector<size_t> & local2global,
  const index_coloring & from_entities,
  std::vector<size_t> & from_ids,
  flecsi::utils::crs & connectivity) {

  int comm_size, comm_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  // the mpi data type for size_t
  const auto mpi_size_t = utils::mpi_typetraits<size_t>::type();

  //----------------------------------------------------------------------------
  // Determine cell-to-vertex connecitivity for my shared cells
  //----------------------------------------------------------------------------

  // first count for storage
  std::vector<size_t> sendcounts(comm_size, 0);
  std::vector<size_t> senddispls(comm_size + 1);

  for(const auto & e : from_entities.shared) {
    // get cell info
    auto start = from2to.offsets[e.offset];
    auto end = from2to.offsets[e.offset + 1];
    auto n = end - start;
    // loop over shared ranks
    for(auto r : e.shared) {
      // we will be sending the cell global id, number of vertices, plus
      // vertices
      if(r != size_t(comm_rank))
        sendcounts[r] += 2 + n;
    }
  }

  // finish displacements
  senddispls[0] = 0;
  for(int r = 0; r < comm_size; ++r)
    senddispls[r + 1] = senddispls[r] + sendcounts[r];

  // now fill buffers
  std::vector<size_t> sendbuf(senddispls[comm_size]);
  std::fill(sendcounts.begin(), sendcounts.end(), 0);

  for(const auto & e : from_entities.shared) {
    // get cell info
    auto start = from2to.offsets[e.offset];
    auto end = from2to.offsets[e.offset + 1];
    auto n = end - start;
    // loop over shared ranks
    for(auto r : e.shared) {
      // we will be sending global id, number of vertices, plus vertices
      if(r != size_t(comm_rank)) {
        // first the id and size
        auto j = senddispls[r] + sendcounts[r];
        sendbuf[j++] = e.id;
        sendbuf[j++] = n;
        for(auto i = start; i < end; ++i) {
          auto local_id = from2to.indices[i];
          auto global_id = local2global[local_id];
          sendbuf[j++] = global_id;
        }
        // increment send counter
        sendcounts[r] = j - senddispls[r];
      }
    } // shared ranks
  }

  //----------------------------------------------------------------------------
  // Send shared information
  //----------------------------------------------------------------------------

  // send the counts
  std::vector<size_t> recvcounts(comm_size);
  auto ret = MPI_Alltoall(sendcounts.data(),
    1,
    mpi_size_t,
    recvcounts.data(),
    1,
    mpi_size_t,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertex counts");

  // how much info will we be receiving
  std::vector<size_t> recvdispls(comm_size + 1);
  recvdispls[0] = 0;
  for(int r = 0; r < comm_size; ++r)
    recvdispls[r + 1] = recvdispls[r] + recvcounts[r];
  std::vector<size_t> recvbuf(recvdispls[comm_size]);

  // now send the actual vertex info
  ret = alltoallv(sendbuf,
    sendcounts,
    senddispls,
    recvbuf,
    recvcounts,
    recvdispls,
    MPI_COMM_WORLD);

  if(ret != MPI_SUCCESS)
    flog_error("Error communicating vertices");

  //----------------------------------------------------------------------------
  // Unpack results
  //----------------------------------------------------------------------------

  connectivity.clear();
  connectivity.offsets.emplace_back(0);
  from_ids.clear();

  for(int r = 0; r < comm_size; ++r) {
    for(size_t i = recvdispls[r]; i < recvdispls[r + 1];) {
      // global id
      auto global_id = recvbuf[i];
      from_ids.emplace_back(global_id);
      i++;
      // num of vertices
      size_t n = recvbuf[i];
      i++;
      // no sift through ghost vertices
      for(size_t j = 0; j < n; ++j, ++i) {
        // local and global ids of the vertex (relative to sender)
        auto ent_global_id = recvbuf[i];
        // Don't bother checking if i have this vertex yet, even if I already
        // have it, it might be a ghost
        connectivity.indices.emplace_back(ent_global_id);
      } // vertices
      // now add final offset
      connectivity.offsets.emplace_back(connectivity.offsets.back() + n);
    }
  }
}

template<std::size_t MESH_DIMENSION>
void
ghost_connectivity(const parallel_definition<MESH_DIMENSION> & md,
  size_t from_dimension,
  size_t to_dimension,
  const index_coloring & from_entities,
  std::vector<size_t> & from_ids,
  flecsi::utils::crs & connectivity) {

  const auto & to_local2global = md.local_to_global(to_dimension);
  const auto & from2to = md.entities_crs(from_dimension, to_dimension);

  ghost_connectivity(
    from2to, to_local2global, from_entities, from_ids, connectivity);
}

} // namespace unstructured_impl
} // namespace topology
} // namespace flecsi
