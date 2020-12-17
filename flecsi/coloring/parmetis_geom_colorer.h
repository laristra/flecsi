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

struct parmetis_geom_colorer_t : public colorer_t {
  /*!
   Default constructor
   */
  template<typename T>
  parmetis_geom_colorer_t(const std::vector<T> & xyz, idx_t ndims)
    : ndims_(ndims), xyz_(xyz.begin(), xyz.end()) {}

  /*!
   Copy constructor (disabled)
   */
  parmetis_geom_colorer_t(const parmetis_geom_colorer_t &) = delete;

  /*!
   Assignment operator (disabled)
   */
  parmetis_geom_colorer_t & operator=(const parmetis_geom_colorer_t &) = delete;

  /*!
    Destructor
   */
  ~parmetis_geom_colorer_t() {}

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

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    auto num_ents = dcrs.size();
    auto vtxdist = dcrs.distribution_as<idx_t>();

    if(num_ents == 0)
      clog_fatal(
        "Unfortunately, ParMETIS_V3_PartGeom requires all ranks to have an "
        "initial partitioning.");

    if(num_parts != size)
      clog_fatal(
        "Unfortunately, ParMETIS_V3_PartGeom requires nparts == comm_size.");

    std::vector<idx_t> part(num_ents, rank);

    // Actual call to ParMETIS.
    idx_t npart = num_parts;
    MPI_Comm comm = MPI_COMM_WORLD;
    int result = ParMETIS_V3_PartGeom(
      &vtxdist[0], &ndims_, xyz_.data(), part.data(), &comm);
    if(result != METIS_OK)
      clog_error("Parmetis failed!");

    std::vector<size_t> partitioning(part.begin(), part.end());
    return partitioning;

  } // color

private:
  idx_t ndims_;
  std::vector<::real_t> xyz_;

}; // struct parmetis_geom_colorer_t

} // namespace coloring
} // namespace flecsi
