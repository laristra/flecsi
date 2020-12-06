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

#include "flecsi/util/mpi.hh"

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

  flog_assert((naive.distribution.size() - 1) == size_t(size),
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

    flecsi::util::color_map cm(size, colors, naive.distribution.back());

    for(size_t i{0}; i < naive.entries(); ++i) {
      part[i] = cm.index_color(naive.distribution[rank] + i);
    } // for
  } // if

  // FIXME: Remove or guard
  std::stringstream ss;
  ss << "part size: " << part.size() << std::endl;
  for(auto i : part) {
    ss << i << " ";
  }
  ss << std::endl;
  flog_devel(info) << ss.str() << std::endl;

  std::vector<idx_t> vtxdist = as<idx_t>(naive.distribution);
  std::vector<idx_t> xadj = as<idx_t>(naive.offsets);
  std::vector<idx_t> adjncy = as<idx_t>(naive.indices);

  // clang-format off
  int result = ParMETIS_V3_PartKway(&vtxdist[0], &xadj[0], &adjncy[0],
    nullptr, nullptr, &wgtflag, &numflag, &ncon, &colors, &tpwgts[0],
    ubvec.data(), options, &edgecut, &part[0], &comm);
  // clang-format on

  flog_assert(result == METIS_OK, "ParMETIS_V3_PartKway returned " << result);

  return {part.begin(), part.end()};
} // color

} // namespace parmetis
} // namespace util
} // namespace flecsi
