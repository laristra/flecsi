/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_dmp_parmetis_partitioner_h
#define flecsi_dmp_parmetis_partitioner_h

#include "flecsi/partition/partitioner.h"

#include <set>

#include <mpi.h>
#include <parmetis.h>

#include "flecsi/partition/mpi_utils.h"

///
// \file parmetis_partitioner.h
// \authors bergen
// \date Initial file creation: Nov 24, 2016
///

namespace flecsi {
namespace dmp {

///
// \class parmetis_partitioner_t parmetis_partitioner.h
// \brief parmetis_partitioner_t provides...
///
class parmetis_partitioner_t
  : public partitioner_t
{
public:

  /// Default constructor
  parmetis_partitioner_t() {}

  /// Copy constructor (disabled)
  parmetis_partitioner_t(const parmetis_partitioner_t &) = delete;

  /// Assignment operator (disabled)
  parmetis_partitioner_t & operator = (const parmetis_partitioner_t &) = delete;

  /// Destructor
  ~parmetis_partitioner_t() {}

  ///
  // Generate a primary partition using the ParMETIS library.
  ///
  std::set<size_t>
  partition(
    dcrs_t & dcrs
  ) override
  {
    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //------------------------------------------------------------------------//
    // Call ParMETIS partitioner.
    //------------------------------------------------------------------------//

    idx_t wgtflag = 0;
    idx_t numflag = 0;
    idx_t ncon = 1;
    std::vector<real_t> tpwgts(size);

    real_t sum = 0.0;
    for(size_t i(0); i<tpwgts.size(); ++i) {
      if(i == (tpwgts.size()-1)) {
        tpwgts[i] = 1.0 - sum;
      }
      else {
        tpwgts[i] = 1.0/size;
        sum += tpwgts[i];
      } // if

      #if 0
      if(rank == 0) {
        std::cout << tpwgts[i] << std::endl;
      } // if
      #endif
    } // for

    // We may need to expose some of the ParMETIS configuration options.
    real_t ubvec = 1.05;
    idx_t options = 0;
    idx_t edgecut;
    MPI_Comm comm = MPI_COMM_WORLD;
    std::vector<idx_t> part(dcrs.size());

    // Get the dCRS information using ParMETIS types.
    std::vector<idx_t> vtxdist = dcrs.distribution_as<idx_t>();
    std::vector<idx_t> xadj = dcrs.offsets_as<idx_t>();
    std::vector<idx_t> adjncy = dcrs.indices_as<idx_t>();

    // Actual call to ParMETIS.
    int result = ParMETIS_V3_PartKway(&vtxdist[0], &xadj[0],
      &adjncy[0], nullptr, nullptr, &wgtflag, &numflag, &ncon, &size,
      &tpwgts[0], &ubvec, &options, &edgecut, &part[0], &comm);

    #if 0
    std::cout << "rank " << rank << ": ";
    for(size_t i(0); i<dcrs.size(); ++i) {
      std::cout << "[" << part[i] << ", " << vtxdist[rank]+i << "] ";
    } // for
    std::cout << std::endl;
    #endif

    //------------------------------------------------------------------------//
    // Exchange information with other ranks.
    //------------------------------------------------------------------------//

    std::vector<idx_t> send_cnts(size, 0);
    std::vector<std::vector<idx_t>> sbuffers;

    std::set<size_t> primary;

    // Find the indices we need to request.
    for(size_t r(0); r<size; ++r) {
      std::vector<idx_t> indices;

      for(size_t i(0); i<dcrs.size(); ++i) {
        if(part[i] == r) {
          indices.push_back(vtxdist[rank] + i);
        }
        else if(part[i] == rank) {
          // If the index belongs to us, just add it...
          primary.insert(vtxdist[rank] + i);
        } // if
      } // for

      sbuffers.push_back(indices);
      send_cnts[r] = indices.size();
    } // for

#if 0
//  if(rank == 0) {
    size_t rcnt(0);
    for(auto r: sbuffers) {
      std::cout << "rank " << rank << " sends " << rcnt++ <<
        " " << send_cnts[rcnt] << ": ";
      for(auto i: r) {
        std::cout << i << " ";
      } // for
      std::cout << std::endl;
    } // for
 // } // if
#endif

    // Do all-to-all to find out where everything belongs.
    std::vector<idx_t> recv_cnts(size);
    result = MPI_Alltoall(&send_cnts[0], 1, mpi_typetraits<idx_t>::type(),
      &recv_cnts[0], 1, mpi_typetraits<idx_t>::type(), MPI_COMM_WORLD);

#if 0
    if(rank == 0) {
      std::cout << "rank " << rank << " receives: ";

      for(size_t i(0); i<size; ++i) {
        std::cout << recv_cnts[i] << " ";
      } // for

      std::cout << std::endl;
    } // if
#endif

    // Start receive operations (non-blocking).
    std::vector<std::vector<idx_t>> rbuffers(size);
    std::vector<MPI_Request> requests;
    for(size_t r(0); r<size; ++r) {
      if(recv_cnts[r]) {
        rbuffers[r].resize(recv_cnts[r]);
        requests.push_back({});
        MPI_Irecv(&rbuffers[r][0], recv_cnts[r], mpi_typetraits<idx_t>::type(),
          r, 0, MPI_COMM_WORLD, &requests[requests.size()-1]);
      } // if
    } // for

    // Start send operations (blocking is probably ok here).
    for(size_t r(0); r<size; ++r) {
      if(send_cnts[r]) {
        sbuffers[r].resize(send_cnts[r]);
        MPI_Send(&sbuffers[r][0], send_cnts[r],
          mpi_typetraits<idx_t>::type(),
          r, 0, MPI_COMM_WORLD);
      } // if
    } // for

    // Wait on the receive operations
    std::vector<MPI_Status> status(requests.size());
    MPI_Waitall(requests.size(), &requests[0], &status[0]);

    // Add indices to primary
    for(size_t r(0); r<size; ++r) {
      if(recv_cnts[r]) {
        for(auto i: rbuffers[r]) {
          primary.insert(i);
        } // for
      } // if
    } // for

  #if 0
  //  if(rank == 0) {
      std::cout << "rank " << rank << " primary partition:" << std::endl;
      for(auto i: primary) {
        std::cout << i << " ";
      } // for
      std::cout << std::endl;
  //  } // if
  #endif

    return primary;
  } // partition

  ///
  //
  ///
  std::pair<std::vector<std::set<size_t>>, std::set<entry_info_t>>
  get_cell_info(
    std::set<size_t> & primary,
    std::set<size_t> & request_indices
  )
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

    // This may be inefficient, but this call is doing a reduction
    // to determine the maximum number of indices requested by any rank
    // so that we can pad out the all-to-all communication below.
    int result = MPI_Allreduce(&request_indices_size, &max_request_indices, 1,
      flecsi::dmp::mpi_typetraits<size_t>::type(), MPI_MAX, MPI_COMM_WORLD);

#if 0
    if(rank == 0) {
      std::cout << "max indices: " << max_request_indices << std::endl;
    } // if
#endif

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

#if 0
      if(rank == 0) {
        std::cout << "rank " << rank << " requests: ";
        for(size_t i(0); i<max_request_indices; ++i) {
          std::cout << input_indices[roff + i] << " ";
        } // for
        std::cout << std::endl;
      } // if
#endif
    } // for

    // Send the request indices to all other ranks.
    result = MPI_Alltoall(&input_indices[0], max_request_indices,
      flecsi::dmp::mpi_typetraits<size_t>::type(),
      &info_indices[0], max_request_indices,
      flecsi::dmp::mpi_typetraits<size_t>::type(), MPI_COMM_WORLD);

    // Reset input indices to use to send back information
    std::fill(input_indices.begin(), input_indices.end(),
      std::numeric_limits<size_t>::max());

    // For the primary partition, provide rank and cell information
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
          // This is a match, i.e., we own this cell, so we can
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
      flecsi::dmp::mpi_typetraits<size_t>::type(),
      &info_indices[0], max_request_indices,
      flecsi::dmp::mpi_typetraits<size_t>::type(), MPI_COMM_WORLD);

    // Send the offsets information back to all ranks.
    result = MPI_Alltoall(&input_offsets[0], max_request_indices,
      flecsi::dmp::mpi_typetraits<size_t>::type(),
      &info_offsets[0], max_request_indices,
      flecsi::dmp::mpi_typetraits<size_t>::type(), MPI_COMM_WORLD);

    std::set<entry_info_t> remote;

    // Collect all of the information for the remote cells.
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
          remote.insert(entry_info_t(request_indices_vector[i], ranks[i],
            offsets[i], {}));
        } // if
      } // for
    } // for

    return std::make_pair(local , remote);
  } // get_info

private:

}; // class parmetis_partitioner_t

} // namespace dmp
} // namespace flecsi

#endif // flecsi_dmp_parmetis_partitioner_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
