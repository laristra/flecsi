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

#include <metis.h>

#include <flecsi/coloring/colorer.h>
#include <flecsi/utils/mpi_type_traits.h>

namespace flecsi {
namespace coloring {

/*!
  The colorer_t type provides a ParMETIS implementation of the
  colorer_t interface.
 */

struct metis_colorer_t : public colorer_t {
  /*!
   Default constructor
   */
  metis_colorer_t() = default;

  /*!
   Copy constructor (disabled)
   */
  metis_colorer_t(const metis_colorer_t &) = delete;

  /*!
   Assignment operator (disabled)
   */
  metis_colorer_t & operator=(const metis_colorer_t &) = delete;

  /*!
    Destructor
   */
  ~metis_colorer_t() {}

  /*!
   Implementation of color method. See \ref colorer_t::color.
   */

  std::set<size_t> color(const dcrs_t & dcrs) override {
    clog_error("Not implemented.");
    return {};
  } // color

  /*!
   Implementation of color method. See \ref colorer_t::color.
   */

  std::vector<size_t> new_color(size_t num_parts,
    const dcrs_t & dcrs) override {

    auto num_ents = dcrs.size();
    auto comm = create_communicator(MPI_COMM_WORLD, num_ents);
    if(!num_ents)
      return {};

    int world_size, comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_size(comm, &comm_size);

    if(num_parts == comm_size) {
      clog_fatal(
        "This partitioner subdivides an existing partitioning. Since "
        " every rank has an initial partition, no subdivision is possible. "
        " Rerun with fewer initial partition files.");
    }

    // subdivide the partition distributions evenly
    std::vector<size_t> part_dist;
    subdivide(num_parts, comm_size, part_dist);

    // how many pieces is this rank to create
    int comm_rank;
    MPI_Comm_rank(comm, &comm_rank);
    auto num_pieces = part_dist[comm_rank + 1] - part_dist[comm_rank];

    // Get the dCRS information using ParMETIS types.  Also convert the global
    // graph to a local one
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    auto vtxstart = dcrs.distribution[world_rank];
    auto vtxend = dcrs.distribution[world_rank + 1];

    std::vector<idx_t> xadj, adjncy;
    xadj.reserve(dcrs.offsets.size());
    adjncy.reserve(dcrs.indices.size());

    xadj.emplace_back(0);
    for(size_t i = 0; i < num_ents; ++i) {
      for(auto j = dcrs.offsets[i]; j < dcrs.offsets[i + 1]; ++j) {
        auto id = dcrs.indices[j];
        if(id >= vtxstart && id < vtxend)
          adjncy.emplace_back(id - vtxstart);
      }
      xadj.emplace_back(adjncy.size());
    }

    // set inputs
    idx_t wgtflag = 0;
    idx_t numflag = 0;
    idx_t ncon = 1;
    std::vector<real_t> tpwgts(ncon * num_pieces, 1.0 / num_pieces);

    // We may need to expose some of the ParMETIS configuration options.
    std::vector<real_t> ubvec(ncon, 1.05);
    idx_t edgecut;

    std::vector<idx_t> part(num_ents);

    // Actual call to ParMETIS.
    idx_t nvtx = num_ents;
    idx_t npart = num_pieces;
    int result =
      METIS_PartGraphKway(&nvtx, &ncon, &xadj[0], &adjncy[0], nullptr, nullptr,
        nullptr, &npart, &tpwgts[0], ubvec.data(), nullptr, &edgecut, &part[0]);
    if(result != METIS_OK)
      clog_error("Parmetis failed!");

    std::vector<size_t> partitioning(part.begin(), part.end());
    for(auto & p : partitioning)
      p += part_dist[comm_rank];

    return partitioning;

  } // color

}; // struct metis_colorer_t

} // namespace coloring
} // namespace flecsi
