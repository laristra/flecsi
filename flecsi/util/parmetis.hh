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

template<typename Definition>
inline util::dcrs
new_naive_coloring(Definition const & md,
  size_t entity_dimension,
  size_t thru_dimension,
  MPI_Comm comm = MPI_COMM_WORLD) {

  // FIXME
  (void)thru_dimension;

  int size, rank;
  MPI_Group group;

  MPI_Comm_size(comm, &size);
  MPI_Comm_group(comm, &group);
  MPI_Group_rank(group, &rank);

  util::dcrs dcrs;

  flog(info) << "cells: " << md.num_entities(2) << std::endl;
  flog(info) << "vertices: " << md.num_entities(0) << std::endl;

  util::color_map cell_map(size, size, md.num_entities(entity_dimension));
  util::color_map vertex_map(size, size, md.num_entities(0));

  std::stringstream ss;
  ss << "cell distribution: ";
  for(auto e : cell_map.distribution()) {
    ss << e << " ";
  }
  flog(info) << ss.str() << std::endl;
  ss.str(std::string{});

  ss << "vertex distribution: ";
  for(auto e : vertex_map.distribution()) {
    ss << e << " ";
  }
  flog(info) << ss.str() << std::endl;
  ss.str(std::string{});

  MPI_Status status;
  MPI_Request * size_requests{nullptr};
  MPI_Request * data_requests;

  if(rank == 0) {
    size_requests = new MPI_Request[size];
    data_requests = new MPI_Request[size];
    for(size_t r{1}; r < std::size_t(size); ++r) {
      const auto start = cell_map.distribution()[r];
      const auto end = cell_map.distribution()[r + 1];
      flog(info) << "process " << r << " range: [" << start << ", " << end
                 << ")" << std::endl;

      std::vector<std::vector<size_t>> cells(end - start);
      for(size_t i{start}; i < end; ++i) {
        cells.emplace_back(md.entities(2, 0, i));
#if 0
        ss << "cell " << i << ": ";
        auto vertices = md.entities(2, 0, i);
        for(auto v : vertices) {
          ss << v << " ";
        } // for
        flog(info) << ss.str() << std::endl;
        ss.str(std::string{});
#endif
      } // for

      auto data = serial_put(cells);

      const std::size_t bytes = data.size();
      flog(info) << "sending " << bytes << " to " << r << std::endl;

      MPI_Isend(
        &bytes, 1, mpi::type<std::size_t>(), r, 0, comm, &size_requests[r]);
      MPI_Isend(data.data(),
        data.size(),
        mpi::type<std::byte>(),
        r,
        0,
        comm,
        &data_requests[r]);
    } // for
  }
  else {
    std::size_t bytes{0};
    MPI_Recv(&bytes, 1, mpi::type<std::size_t>(), 0, 0, comm, &status);
    flog(info) << "set to receive " << bytes << " bytes" << std::endl;
    std::vector<std::byte> data(bytes);
    MPI_Recv(data.data(), bytes, mpi::type<std::byte>(), 0, 0, comm, &status);
    const auto * p = data.data();
    auto cells = serial_get<std::vector<std::vector<size_t>>>(p);

    std::size_t i{0};
    for(auto c : cells) {
      ss << "cell " << cell_map.distribution()[rank - 1] + i++ << " ";
      for(auto v : c) {
        ss << v << " ";
      } // for
      flog(info) << ss.str() << std::endl;
      ss.str(std::string{});
    } // for

  } // if

  if(rank == 0) {
    for(size_t r{1}; r < std::size_t(size); ++r) {
      MPI_Wait(&size_requests[r], &status);
      MPI_Wait(&data_requests[r], &status);
    } // for

    delete[] size_requests;
    delete[] data_requests;
  } // if

  return dcrs;
} // new_naive_coloring

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
