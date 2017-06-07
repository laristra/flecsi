/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_coloring_mpi_communicator_h
#define flecsi_coloring_mpi_communicator_h

///
/// \file
/// \date Initial file creation: Dec 06, 2016
///

#include "flecsi/coloring/communicator.h"

//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>
#include <cinchlog.h>

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include "flecsi/utils/set_utils.h"

clog_register_tag(mpi_communicator);

namespace flecsi {
namespace coloring {

///
/// \class mpi_communicator_t mpi_communicator.h
/// \brief mpi_communicator_t provides an implementation of the
///        communicator_t interface using MPI.
///
class mpi_communicator_t
  : public communicator_t
{
public:

  /// Default constructor
  mpi_communicator_t() {}

  /// Copy constructor (disabled)
  mpi_communicator_t(const mpi_communicator_t &) = delete;

  /// Assignment operator (disabled)
  mpi_communicator_t & operator = (const mpi_communicator_t &) = delete;

  /// Destructor
   ~mpi_communicator_t() {}

  ///
  /// Rerturn a set containing the entity_info_t information for each
  /// member of the input set request_indices (from other ranks) and
  /// the information for the local indices in primary.
  ///
  std::pair<std::vector<std::set<size_t>>, std::set<entity_info_t>>
  get_primary_info(
    const std::set<size_t> & primary,
    const std::set<size_t> & request_indices
  )
  override
  {
    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Store the request in a vector for indexed access below.
    std::vector<size_t> request_indices_vector(request_indices.begin(),
      request_indices.end());

    size_t request_indices_size = request_indices.size();
    size_t max_request_indices(0);

#if 0
    std::cout << "rank " << rank << " indices set: " <<
      request_indices_size << std::endl;
#endif

    const auto mpi_size_t_type =
      flecsi::coloring::mpi_typetraits<size_t>::type();

    // This may be inefficient, but this call is doing a reduction
    // to determine the maximum number of indices requested by any rank
    // so that we can pad out the all-to-all communication below.
    int result = MPI_Allreduce(&request_indices_size, &max_request_indices, 1,
      mpi_size_t_type, MPI_MAX, MPI_COMM_WORLD);

    // Pad the request indices with size_t max. We will then set
    // the indices of the actual request. Each rank that receives
    // the request will try to provide information about the
    // non size_t max values in the request. The others will
    // be ignored.
    std::vector<size_t> input_indices(size*max_request_indices,
      std::numeric_limits<size_t>::max());
    std::vector<size_t> info_indices(size*max_request_indices);

    // For now, we need two arrays for each all-to-all communication:
    // One for rank ownership of the request indices, and one
    // for the offsets. We could probably combine these. However,
    // we would probably have to define a custom MPI type. It
    // will only be worth the effort if this appraoch is slow.
    // The input offsets do not need to be initialized because
    // the information is available in the input_indices array.
    std::vector<size_t> input_offsets(size*max_request_indices);
    std::vector<size_t> info_offsets(size*max_request_indices);

    // Populate the request vectors for each rank.
    for(size_t r(0); r<size; ++r) {

      size_t off(0);
      const size_t roff = r*max_request_indices;

      // Set the actual indices of the request
      for(auto i: request_indices) {
        input_indices[roff + off++] = i;
      } // for

    } // for

    // Send the request indices to all other ranks.
    result = MPI_Alltoall(&input_indices[0], max_request_indices,
      flecsi::coloring::mpi_typetraits<size_t>::type(),
      &info_indices[0], max_request_indices,
      flecsi::coloring::mpi_typetraits<size_t>::type(), MPI_COMM_WORLD);

    // Reset input indices to use to send back information
    std::fill(input_indices.begin(), input_indices.end(),
      std::numeric_limits<size_t>::max());

    // For the primary coloring, provide rank and entity information
    // on indices that are shared with other processes.
    std::vector<std::set<size_t>> local(primary.size());

    // See if we can fill any requests...
    for(size_t r(0); r<size; ++r) {

      // Ignore our rank
      if(r == rank) {
        continue;
      } // if

      // These array slices are just for convenience.
      size_t * info = &info_indices[r*max_request_indices];
      size_t * offset = &input_offsets[r*max_request_indices];
      size_t * input = &input_indices[r*max_request_indices];

      // See which requests we can fulfill.
      for(size_t i(0); i<max_request_indices; ++i) {

        auto match = primary.find(info[i]);

        if(match != primary.end()) {
          // This is a match, i.e., we own this entity, so we can
          // set the rank (ownership) and offset.
          input[i] = rank;
          offset[i] = std::distance(primary.begin(), match);

          // We also need to register that this index is shared
          // with other ranks
          local[offset[i]].insert(r);
        } // if
      } // for
    } // for

#if 0
    size_t cnt(0);
    for(auto i: local) {
      std::cout << "index: " << cnt++ << " shares ";
      for(auto r: i) {
        std::cout << r << " ";
      } // for
      std::cout << std::endl;
    } // for
#endif

    // Send the indices information back to all ranks.
    result = MPI_Alltoall(&input_indices[0], max_request_indices,
      mpi_size_t_type, &info_indices[0], max_request_indices,
      mpi_size_t_type, MPI_COMM_WORLD);

    // Send the offsets information back to all ranks.
    result = MPI_Alltoall(&input_offsets[0], max_request_indices,
      mpi_size_t_type, &info_offsets[0], max_request_indices,
      mpi_size_t_type, MPI_COMM_WORLD);

    std::set<entity_info_t> remote;

    // Collect all of the information for the remote entities.
    for(size_t r(0); r<size; ++r) {
      // Skip these (we already know them!)
      if(r == rank) {
        continue;
      } // if

      // Another slice for convenience.
      size_t * ranks = &info_indices[r*max_request_indices];
      size_t * offsets = &info_offsets[r*max_request_indices];

      for(size_t i(0); i<max_request_indices; ++i) {

        if(ranks[i] != std::numeric_limits<size_t>::max()) {
          // If this is not size_t max, this rank answered our request
          // and we can set the information.
          remote.insert(entity_info_t(request_indices_vector[i], ranks[i],
            offsets[i], {}));
        } // if
      } // for
    } // for

    return std::make_pair(local , remote);
  } // get_primary_info

  std::unordered_map<size_t, std::set<size_t>>
  get_intersection_info(
    const std::set<size_t> & request_indices
  )
  override
  {
    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Store the request in a vector for indexed access below.
    std::vector<size_t> request_indices_vector(request_indices.begin(),
      request_indices.end());

    size_t request_indices_size = request_indices.size();
    size_t max_request_indices(0);

    const auto mpi_size_t_type =
      flecsi::coloring::mpi_typetraits<size_t>::type();

    // This may be inefficient, but this call is doing a reduction
    // to determine the maximum number of indices requested by any rank
    // so that we can pad out the all-to-all communication below.
    int result = MPI_Allreduce(&request_indices_size, &max_request_indices, 1,
      mpi_size_t_type, MPI_MAX, MPI_COMM_WORLD);

    // Pad the request indices with size_t max. We will then set
    // the indices of the actual request. Each rank that receives
    // the request will try to provide information about the
    // non size_t max values in the request. The others will
    // be ignored.
    std::vector<size_t> input_indices(size*max_request_indices,
      std::numeric_limits<size_t>::max());
    std::vector<size_t> info_indices(size*max_request_indices);

    // For now, we need two arrays for each all-to-all communication:
    // One for rank ownership of the request indices, and one
    // for the offsets. We could probably combine these. However,
    // we would probably have to define a custom MPI type. It
    // will only be worth the effort if this appraoch is slow.
    // The input offsets do not need to be initialized because
    // the information is available in the input_indices array.
    std::vector<size_t> input_offsets(size*max_request_indices);
    std::vector<size_t> info_offsets(size*max_request_indices);

    // Populate the request vectors for each rank.
    for(size_t r(0); r<size; ++r) {

      size_t off(0);
      const size_t roff = r*max_request_indices;

      // Set the actual indices of the request
      for(auto i: request_indices) {
        input_indices[roff + off++] = i;
      } // for
    } // for

    // Send the request indices to all other ranks.
    result = MPI_Alltoall(&input_indices[0], max_request_indices,
      mpi_size_t_type, &info_indices[0], max_request_indices,
      mpi_size_t_type, MPI_COMM_WORLD);

    // Reset input indices to use to send back information
    std::fill(input_indices.begin(), input_indices.end(),
      std::numeric_limits<size_t>::max());

    {
    clog_tag_guard(mpi_communicator);
    clog_container_one(info, "input_indices", info_indices, clog::space);
    }

    //
    std::unordered_map<size_t, std::set<size_t>> intersection_map;

    for(size_t r(0); r<size; ++r) {

      // Ignore our rank
      if(r == rank) {
        continue;
      } // if

      // Array slice for convenience.
      size_t * info = &info_indices[r*max_request_indices];

      // Create a set of the off-color request indices.
      std::set<size_t> intersection_set;
      for(size_t i(0); i<max_request_indices; ++i) {
        if(info[i] != std::numeric_limits<size_t>::max()) {
          intersection_set.insert(info[i]);
        } // if
      } // for

      {
      clog_tag_guard(mpi_communicator);
      clog_container_one(info, "intersection_set", intersection_set,
        clog::space);
      }

      // Compute the intersection
      auto intersection = flecsi::utils::set_intersection(intersection_set,
        request_indices);

      {
      clog_tag_guard(mpi_communicator);
      clog_container_one(info,
        "rank " << r << " intersection", intersection, clog::space);
      }

      // If the intersection is non-empty, add it to the return map
      if(intersection.size()) {
        intersection_map[r] = intersection;
      } // if
    } // for

    return intersection_map;
  } // get_intersection_info

  ///
  /// Rerturn a set containing the entity_info_t information for each
  /// member of the input set request_indices (from other ranks).
  ///
  /// \param entity_info FIXME...
  /// \param request_indices A set of entity ids for which to return
  ///                        information.
  /// \return A std::vector<std::set<size_t>> containing the offset
  ///         information for the requested indices.
  ///
  std::vector<std::set<size_t>>
  get_entity_info(
    const std::set<entity_info_t> & entity_info,
    const std::vector<std::set<size_t>> & request_indices
  )
  override
  {
    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Collect the size of each rank request to send.
    std::vector<size_t> send_cnts(size, 0);
    for(size_t r(0); r<size; ++r) {
      send_cnts[r] = request_indices[r].size();
    } // for

    // Send the request size (in indices) to each rank.
    std::vector<size_t> recv_cnts(size);
    int result = MPI_Alltoall(&send_cnts[0], 1, mpi_typetraits<size_t>::type(),
    &recv_cnts[0], 1, mpi_typetraits<size_t>::type(), MPI_COMM_WORLD);

#if 0
//    if(rank == 0) {
      std::cout << "rank " << rank << " recieves:" << std::endl;
      for(auto i: recv_cnts) {
        std::cout << i << " ";
      } // for
      std::cout << std::endl;
//    } // if
#endif

    // Start receive operations (non-blocking).
    std::vector<std::vector<size_t>> rbuffers(size);
    std::vector<MPI_Request> requests;
    for(size_t r(0); r<size; ++r) {
      if(recv_cnts[r]) {
        rbuffers[r].resize(recv_cnts[r]);
        requests.push_back({});
        MPI_Irecv(&rbuffers[r][0], recv_cnts[r], mpi_typetraits<size_t>::type(),
          r, 0, MPI_COMM_WORLD, &requests[requests.size()-1]);
      } // if
    } // for

    // Start send operations (blocking is ok here).
    std::vector<std::vector<size_t>> sbuffers(size);
    for(size_t r(0); r<size; ++r) {
      if(send_cnts[r]) {
        std::copy(request_indices[r].begin(), request_indices[r].end(),
          std::back_inserter(sbuffers[r]));

#if 0
        std::cout << "rank " << rank << " sends " << r << ": ";
        for(auto i: sbuffers[r]) {
          std::cout << i << " ";
        } // for
        std::cout << std::endl;
#endif

        MPI_Send(&sbuffers[r][0], send_cnts[r], mpi_typetraits<size_t>::type(),
          r, 0, MPI_COMM_WORLD);
      } // if
    } // for

    // Create a map version of the entity info for lookups below.
    std::unordered_map<size_t, entity_info_t> entity_info_map;
    for(auto i: entity_info) {
      entity_info_map[i.id] = i;
    } // for

    // Wait on the receive operations
    std::vector<MPI_Status> status(requests.size());
    MPI_Waitall(requests.size(), &requests[0], &status[0]);

#if 0
if(rank == 0) {
    std::cout << "rank " << rank << " received:" << std::endl;
    for(size_t r(0); r<size; ++r) {
      for(auto i: rbuffers[r]) {
        std::cout << i << " ";
      } // for
      std::cout << std::endl;
    } // for
} // if
#endif

    // Set the offsets for each requested index in the send buffer.
    for(size_t r(0); r<size; ++r) {
      sbuffers[r].resize(rbuffers[r].size());

      size_t offset(0);
      for(auto i: rbuffers[r]) {
        sbuffers[r][offset++] = entity_info_map[i].offset;
      } // for
    } // for

#if 0
if(rank == 0) {
    std::cout << "rank " << rank << " provides:" << std::endl;
    for(size_t r(0); r<size; ++r) {
      for(auto i: sbuffers[r]) {
        std::cout << i << " ";
      } // for
      std::cout << std::endl;
    } // for
} // if
#endif
    // we are reusing requests in the following code, need to clear the content
    // first.
    requests.clear();

    // Start receive operations (non-blocking) to get back the
    // offsets we requested.
    for(size_t r(0); r<size; ++r) {
      // If we sent a request, prepare to receive an answer.
      if(send_cnts[r]) {
        // We're done with our receive buffers, so we can re-use them.
        rbuffers[r].resize(send_cnts[r], 0);
        requests.push_back({});
        MPI_Irecv(&rbuffers[r][0], send_cnts[r], mpi_typetraits<size_t>::type(),
          r, 0, MPI_COMM_WORLD, &requests[requests.size()-1]);
      } // if
    } // for

    // Start send operations (blocking is probably ok here).
    for(size_t r(0); r<size; ++r) {
      // If we received a request, prepare to send an answer.
      if(recv_cnts[r]) {
        MPI_Send(&sbuffers[r][0], recv_cnts[r], mpi_typetraits<size_t>::type(),
          r, 0, MPI_COMM_WORLD);
      } // if
    } // for

    // Wait on the receive operations
    status.resize(requests.size());
    MPI_Waitall(requests.size(), &requests[0], &status[0]);

    std::vector<std::set<size_t>> remote(size);
    for(size_t r(0); r<size; ++r) {
      for(size_t i(0); i<send_cnts[r]; ++i) {
        remote[r].insert(rbuffers[r][i]);
      } // for
    } // for

#if 0
if(rank == 1) {
    std::cout << "remote: " << std::endl;
    size_t r(0);
    for(auto i: remote) {
      std::cout << "rank " << r++ << ": ";
      for(auto s: i) {
        std::cout << s << " ";
      } // for
      std::cout << std::endl;
    } // for
} // if
#endif

    return remote;
  } // get_entity_info

  ///
  /// Rerturn a map containing the coloring index and the number of indices
  /// for the given index set.
  ///     
  std::unordered_map<size_t, size_t>
  gather_sizes(
    const size_t & size
  )
  override
  {
    int colors;

    MPI_Comm_size(MPI_COMM_WORLD, &colors);

    std::unordered_map<size_t, size_t> indices_map;
    size_t buffer[colors];

    const auto mpi_size_t_type =
      flecsi::coloring::mpi_typetraits<size_t>::type();

    int result = MPI_Allgather(&size, 1, mpi_size_t_type,
      &buffer, 1, mpi_size_t_type, MPI_COMM_WORLD);

   for (size_t i=0; i<colors; i++)
     indices_map[i]=buffer[i];

    return indices_map;
  } // gather_sizes

  std::unordered_map<size_t, coloring_info_t>
  get_coloring_info(const coloring_info_t & color_info)
  override
  {
    int color, colors;

    MPI_Comm_rank(MPI_COMM_WORLD, &color);
    MPI_Comm_size(MPI_COMM_WORLD, &colors);

    struct size_info_t {
      size_t exclusive;
      size_t shared;
      size_t ghost;
    }; // struct size_info_t

    size_info_t buffer[colors];
    const size_t bytes = sizeof(size_info_t);

    int result = MPI_Allgather(&color_info, bytes, MPI_BYTE,
      &buffer, bytes, MPI_BYTE, MPI_COMM_WORLD);

    std::unordered_map<size_t, coloring_info_t> coloring_info;

    for(size_t c(0); c<colors; ++c) {
      coloring_info[c].exclusive =  buffer[c].exclusive;
      coloring_info[c].shared =  buffer[c].shared;
      coloring_info[c].ghost =  buffer[c].ghost;
    } // for

// FIXME: This pattern gets repeated several times in this file -> Need
//        to create a function to handle it.
    {
    size_t max_request_indices =
      get_max_request_size(color_info.shared_users.size());

    std::cout << "max_request_indices: " << max_request_indices << std::endl;
    
    std::vector<size_t> input_indices(colors*max_request_indices,
      std::numeric_limits<size_t>::max());
    std::vector<size_t> info_indices(colors*max_request_indices);

    for(size_t c(0); c<colors; ++c) {
      size_t off(0);
      const size_t coff = c*max_request_indices;

      for(auto s: color_info.shared_users) {
        input_indices[coff + off++] = s;
      } // for
    } // for

    const auto mpi_size_t_type =
      flecsi::coloring::mpi_typetraits<size_t>::type();

    // Send the request indices to all other ranks.
    result = MPI_Alltoall(
      &input_indices[0], max_request_indices, mpi_size_t_type,
      &info_indices[0], max_request_indices, mpi_size_t_type,
      MPI_COMM_WORLD);

    for(size_t c(0); c<colors; ++c) {

      size_t * info = &info_indices[c*max_request_indices];
      auto & color_info = coloring_info[c];

      for(size_t i(0); i<max_request_indices; ++i) {
        if(info[i] != std::numeric_limits<size_t>::max()) {
          color_info.shared_users.insert(info[i]);
        } // if
      } // for
    } // for
    } // scope
// Pattern ends here

// Repeat
    {
    int max_request_indices =
      get_max_request_size(color_info.ghost_owners.size());

    std::cout << "max_request_indices: " << max_request_indices << std::endl;
    
    std::vector<size_t> input_indices(colors*max_request_indices,
      std::numeric_limits<size_t>::max());
    std::vector<size_t> info_indices(colors*max_request_indices);

    for(size_t c(0); c<colors; ++c) {
      size_t off(0);
      const size_t coff = c*max_request_indices;

      for(auto s: color_info.ghost_owners) {
        input_indices[coff + off++] = s;
      } // for
    } // for

    const auto mpi_size_t_type =
      flecsi::coloring::mpi_typetraits<size_t>::type();

    // Send the request indices to all other ranks.
    result = MPI_Alltoall(
      &input_indices[0], max_request_indices, mpi_size_t_type,
      &info_indices[0], max_request_indices, mpi_size_t_type,
      MPI_COMM_WORLD);

    for(size_t c(0); c<colors; ++c) {

      size_t * info = &info_indices[c*max_request_indices];
      auto & color_info = coloring_info[c];

      for(size_t i(0); i<max_request_indices; ++i) {
        if(info[i] != std::numeric_limits<size_t>::max()) {
          color_info.ghost_owners.insert(info[i]);
        } // if
      } // for
    } // for
    } // scope
// Pattern ends here

    return coloring_info;
  } // get_coloring_info

  size_t
  get_max_request_size(
    size_t request_indices
  )
  {
    size_t max_request_indices = 0;

    // Get a valid MPI type for size_t.
    const auto mpi_size_t_type =
      flecsi::coloring::mpi_typetraits<size_t>::type();

    // This may be inefficient, but this call is doing a reduction
    // to determine the maximum number of indices requested by any rank
    // so that we can pad out the all-to-all communication below.
    int result = MPI_Allreduce(&request_indices, &max_request_indices, 1,
      mpi_size_t_type, MPI_MAX, MPI_COMM_WORLD);

    return max_request_indices;
  } // get_max_request_size

private:

}; // class mpi_communicator_t

} // namespace coloring
} // namespace flecsi

#endif // flecsi_coloring_mpi_communicator_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
