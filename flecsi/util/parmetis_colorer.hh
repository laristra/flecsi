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

#include "flecsi/util/mpi_typetraits.hh"

#if !defined(FLECSI_ENABLE_PARMETIS)
#error FLECSI_ENABLE_PARMETIS not defined! This file depends on ParMETIS!
#endif

#include <parmetis.h>

#include <vector>

namespace flecsi {
namespace util {
namespace parmetis {

static std::vector<size_t>
color(dcrs const & naive, idx_t colors, MPI_Comm comm = MPI_COMM_WORLD) {

  int size, rank;
  MPI_Group group;

  MPI_Comm_size(comm, &size);
  MPI_Comm_group(comm, &group);
  MPI_Group_rank(group, &rank);

  flog_assert(naive.global_colors() == size_t(size),
    "invalid naive coloring! naive.colors("
      << colors << ") must equal comm size(" << size << ")");

  idx_t wgtflag = 0;
  idx_t numflag = 0;
  idx_t ncon = 1;
  std::vector<real_t> tpwgts(ncon * colors, 1.0 / colors);

  // We may need to expose some of the ParMETIS configuration options.
  std::vector<real_t> ubvec(ncon, 1.05);
  idx_t options[4] = {0, 0, 0, 0};
  idx_t edgecut;

  std::vector<idx_t> part(naive.entries());

  if(size != colors) {
    options[0] = 1;
    options[3] = PARMETIS_PSR_UNCOUPLED;

    flecsi::util::color_map cm(size, colors, naive.global_indices());

    for(size_t i{0}; i < naive.entries(); ++i) {
      part[i] = cm.index_color(naive.distribution[rank] + i);
    } // for
  } // if

  std::stringstream ss;
  ss << "part size: " << part.size() << std::endl;
  for(auto i : part) {
    ss << i << " ";
  }
  ss << std::endl;
  flog_devel(info) << ss.str() << std::endl;

  std::vector<idx_t> vtxdist = naive.distribution_as<idx_t>();
  std::vector<idx_t> xadj = naive.offsets_as<idx_t>();
  std::vector<idx_t> adjncy = naive.indices_as<idx_t>();

  // clang-format off
  int result = ParMETIS_V3_PartKway(&vtxdist[0], &xadj[0], &adjncy[0],
    nullptr, nullptr, &wgtflag, &numflag, &ncon, &colors, &tpwgts[0],
    ubvec.data(), options, &edgecut, &part[0], &comm);
  // clang-format on

  flog_assert(result == METIS_OK, "ParMETIS_V3_PartKway returned " << result);

  return {part.begin(), part.end()};
} // color

static std::vector<std::vector<size_t>>
distribute(dcrs const & naive,
  size_t colors,
  std::vector<size_t> const & index_colors,
  MPI_Comm comm = MPI_COMM_WORLD) {
  int size, rank;
  MPI_Group group;

  MPI_Comm_size(comm, &size);
  MPI_Comm_group(comm, &group);
  MPI_Group_rank(group, &rank);

  std::vector<size_t> send_cnts(size, 0);
  std::vector<std::vector<std::array<size_t, 2>>> sbuffers;

  flecsi::util::color_map cm(size, colors, naive.global_indices());

  // Sort out the indices that we need to request.
  for(size_t r{0}; r < size_t(size); ++r) {
    std::vector<std::array<size_t, 2>> indices;

    for(size_t i{0}; i < naive.entries(); ++i) {
      if(cm.process(index_colors[i]) == r) {
        indices.push_back({index_colors[i], naive.distribution[rank] + i});
      } // if
    } // for

    sbuffers.emplace_back(indices);
    send_cnts[r] = r == size_t(rank) ? 0 : indices.size();
  } // for

  std::stringstream ss;

  ss << "send cnts:" << std::endl;
  for(auto sc : send_cnts) {
    ss << sc << " ";
  } // for
  flog_devel(info) << ss.str() << std::endl;

  // Do all-to-all to find out where everything belongs.
  std::vector<size_t> recv_cnts(size);
  auto mpi_size_t = util::mpi_typetraits<size_t>::type();
  int result = MPI_Alltoall(
    &send_cnts[0], 1, mpi_size_t, &recv_cnts[0], 1, mpi_size_t, comm);

  flog_assert(result == MPI_SUCCESS, "MPI_Alltoall returned " << result);

  std::vector<std::vector<std::array<size_t, 2>>> rbuffers(size);
  std::vector<MPI_Request> requests;

  for(size_t r{0}; r < size_t(size); ++r) {
    if(recv_cnts[r]) {
      rbuffers[r].resize(recv_cnts[r]);
      requests.push_back({});
      MPI_Irecv(&rbuffers[r][0],
        recv_cnts[r] * 2,
        mpi_size_t,
        r,
        0,
        comm,
        &requests[requests.size() - 1]);
    } // if
  } // for

  for(size_t r{0}; r < size_t(size); ++r) {
    if(send_cnts[r]) {
      sbuffers[r].resize(send_cnts[r]);
      MPI_Send(&sbuffers[r][0], send_cnts[r] * 2, mpi_size_t, r, 0, comm);
    } // if
  } // for

  std::vector<MPI_Status> status(requests.size());
  MPI_Waitall(requests.size(), &requests[0], &status[0]);

  std::vector<std::vector<size_t>> primaries(cm.colors(rank));

  // Add the indices that we already had locally
  const size_t offset = cm.color_offset(rank);
  for(auto a : sbuffers[rank]) {
    primaries[a[0] - offset].push_back(a[1]);
  } // for

  for(size_t r{0}; r < size_t(size); ++r) {
    if(recv_cnts[r]) {
      for(auto a : rbuffers[r]) {
        primaries[a[0] - offset].push_back(a[1]);
      } // for
    } // if
  } // for

  return primaries;
} // distribute

} // namespace parmetis
} // namespace util
} // namespace flecsi
