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

#include <set>

#include <cinchlog.h>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_PARMETIS)
#error FLECSI_ENABLE_PARMETIS not defined! This file depends on ParMETIS!
#endif

#include <parmetis.h>

#include <flecsi/coloring/colorer.h>
#include <flecsi/utils/mpi_type_traits.h>

namespace flecsi {
namespace coloring {

/*!
  The colorer_t type provides a ParMETIS implementation of the
  colorer_t interface.
 */

struct parmetis_colorer_t : public colorer_t {
  /*!
   Default constructor
   */
  parmetis_colorer_t() {}

  /*!
   Copy constructor (disabled)
   */
  parmetis_colorer_t(const parmetis_colorer_t &) = delete;

  /*!
   Assignment operator (disabled)
   */
  parmetis_colorer_t & operator=(const parmetis_colorer_t &) = delete;

  /*!
    Destructor
   */
  ~parmetis_colorer_t() {}

  /*!
   Implementation of color method. See \ref colorer_t::color.
   */

  std::set<size_t> color(const dcrs_t & dcrs) override {
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
    for(size_t i(0); i < tpwgts.size(); ++i) {
      if(i == (tpwgts.size() - 1)) {
        tpwgts[i] = static_cast<real_t>(1.0 - sum);
      }
      else {
        tpwgts[i] = static_cast<real_t>(1.0 / size);
        sum += tpwgts[i];
      } // if

#if 0
      if(rank == 0) {
        std::cout << tpwgts[i] << std::endl;
      } // if
#endif
    } // for

    // We may need to expose some of the ParMETIS configuration options.
    real_t ubvec = static_cast<real_t>(1.05);
    idx_t options = 0;
    idx_t edgecut;
    MPI_Comm comm = MPI_COMM_WORLD;
    std::vector<idx_t> part(dcrs.size(), (std::numeric_limits<idx_t>::max)());

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
    int result = ParMETIS_V3_PartKway(&vtxdist[0], &xadj[0], &adjncy[0],
      nullptr, nullptr, &wgtflag, &numflag, &ncon, &size, &tpwgts[0], &ubvec,
      &options, &edgecut, &part[0], &comm);

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
    for(size_t r(0); r < size; ++r) {
      std::vector<idx_t> indices;

      for(size_t i(0); i < dcrs.size(); ++i) {
        if(part[i] == r) {
          indices.push_back(static_cast<idx_t>(vtxdist[rank] + i));
        }
        else if(part[i] == rank) {
          // If the index belongs to us, just add it...
          primary.insert(vtxdist[rank] + i);
        } // if
      } // for

      sbuffers.push_back(indices);
      send_cnts[r] = static_cast<idx_t>(indices.size());
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
    result = MPI_Alltoall(&send_cnts[0], 1,
      utils::mpi_typetraits_u<idx_t>::type(), &recv_cnts[0], 1,
      utils::mpi_typetraits_u<idx_t>::type(), MPI_COMM_WORLD);

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
    for(size_t r(0); r < size; ++r) {
      if(recv_cnts[r]) {
        rbuffers[r].resize(recv_cnts[r]);
        requests.push_back({});
        MPI_Irecv(&rbuffers[r][0], static_cast<int>(recv_cnts[r]),
          utils::mpi_typetraits_u<idx_t>::type(), static_cast<int>(r), 0,
          MPI_COMM_WORLD, &requests[requests.size() - 1]);
      } // if
    } // for

    // Start send operations (blocking is probably ok here).
    for(size_t r(0); r < size; ++r) {
      if(send_cnts[r]) {
        sbuffers[r].resize(send_cnts[r]);
        MPI_Send(&sbuffers[r][0], static_cast<int>(send_cnts[r]),
          utils::mpi_typetraits_u<idx_t>::type(), static_cast<int>(r), 0,
          MPI_COMM_WORLD);
      } // if
    } // for

    // Wait on the receive operations
    std::vector<MPI_Status> status(requests.size());
    MPI_Waitall(static_cast<int>(requests.size()), &requests[0], &status[0]);

    // Add indices to primary
    for(size_t r(0); r < size; ++r) {
      if(recv_cnts[r]) {
        for(auto i : rbuffers[r]) {
          primary.insert(i);
        } // for
      } // if
    } // for

#if 0
      if(rank == 0) {
        std::cout << "rank " << rank << " primary coloring:" << std::endl;
        for(auto i: primary) {
          std::cout << i << " ";
        } // for
        std::cout << std::endl;
      } // if
#endif

    clog_assert(primary.size() > 0,
      "At least one rank has an empty primary coloring. Please either "
      "increase the problem size or use fewer ranks");

    return primary;
  } // color

  /*!
   Implementation of color method. See \ref colorer_t::color.
   */

  std::vector<size_t> new_color(const dcrs_t & dcrs) override {
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
    std::vector<real_t> tpwgts(ncon * size, static_cast<real_t>(1.0 / size));

    // We may need to expose some of the ParMETIS configuration options.
    std::vector<real_t> ubvec(ncon, static_cast<real_t>(1.05));
    idx_t options[3] = {0, 0, 0};
    idx_t edgecut;
    MPI_Comm comm = MPI_COMM_WORLD;
    std::vector<idx_t> part(dcrs.size(), (std::numeric_limits<idx_t>::max)());

    // Get the dCRS information using ParMETIS types.
    std::vector<idx_t> vtxdist = dcrs.distribution_as<idx_t>();
    std::vector<idx_t> xadj = dcrs.offsets_as<idx_t>();
    std::vector<idx_t> adjncy = dcrs.indices_as<idx_t>();

    // Actual call to ParMETIS.
    int result = ParMETIS_V3_PartKway(&vtxdist[0], &xadj[0], &adjncy[0],
      nullptr, nullptr, &wgtflag, &numflag, &ncon, &size, &tpwgts[0],
      ubvec.data(), options, &edgecut, &part[0], &comm);
    if(result != METIS_OK) {
      clog_error("Parmetis failed!");
    }

    std::vector<size_t> partitioning(part.begin(), part.end());

    return partitioning;

  } // color

}; // struct parmetis_colorer_t

} // namespace coloring
} // namespace flecsi
