/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_partition_parmetis_partitioner_h
#define flecsi_partition_parmetis_partitioner_h

#include "flecsi/partition/partitioner.h"

#include <set>

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#if !defined(ENABLE_PARMETIS)
  #error ENABLE_PARMETIS not defined! This file depends on ParMETIS!
#endif

#include <mpi.h>
#include <parmetis.h>

#include "flecsi/partition/mpi_utils.h"

///
/// \file
/// \date Initial file creation: Nov 24, 2016
///

namespace flecsi {
namespace dmp {

///
/// \class partitioner_t parmetis_partitioner.h
/// \brief partitioner_t provides a ParMETIS implementation of the
///        partitioner_t interface.
///
struct parmetis_partitioner_t
  : public partitioner_t
{
  /// Default constructor
  parmetis_partitioner_t() {}

  /// Copy constructor (disabled)
  parmetis_partitioner_t(const parmetis_partitioner_t &) = delete;

  /// Assignment operator (disabled)
  parmetis_partitioner_t & operator = (const parmetis_partitioner_t &) = delete;

  /// Destructor
  ~parmetis_partitioner_t() {}

  ///
  /// Implementation of partition method. See \ref partitioner_t::partition.
  ///
  std::set<size_t>
  partition(
    const dcrs_t & dcrs
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
    std::vector<idx_t> part(dcrs.size(), std::numeric_limits<idx_t>::max());

#if 0
    const size_t output_rank(1);
    if(rank == output_rank) {
      std::cout << "rank " << rank << " dcrs: " << std::endl;
      std::cout << "size: " << dcrs.size() << std::endl;
      std::cout << dcrs << std::endl;
    } // if
#endif

//    std::set<size_t> ret;
//    return ret;

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
    if(rank == 0) {
      size_t rcnt(0);
      for(auto r: sbuffers) {
        std::cout << "rank " << rank << " sends " << rcnt++ <<
          " " << send_cnts[rcnt] << ": ";
        for(auto i: r) {
          std::cout << i << " ";
        } // for
        std::cout << std::endl;
      } // for
    } // if
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
      if(rank == 0) {
        std::cout << "rank " << rank << " primary partition:" << std::endl;
        for(auto i: primary) {
          std::cout << i << " ";
        } // for
        std::cout << std::endl;
      } // if
  #endif

    return primary;
  } // partition

}; // struct parmetis_partitioner_t

} // namespace dmp
} // namespace flecsi

#endif // flecsi_partition_parmetis_partitioner_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
