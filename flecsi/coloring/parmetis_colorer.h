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

  std::vector<size_t> color(const dcrs_t & dcrs) override {
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
    std::vector<real_t> tpwgts(ncon*size, 1.0 / size);

    // We may need to expose some of the ParMETIS configuration options.
    std::vector<real_t> ubvec(ncon, 1.05);
    idx_t options[3] = {0, 0, 0};
    idx_t edgecut;
    MPI_Comm comm = MPI_COMM_WORLD;
    std::vector<idx_t> part(dcrs.size(), std::numeric_limits<idx_t>::max());

    // Get the dCRS information using ParMETIS types.
    std::vector<idx_t> vtxdist = dcrs.distribution_as<idx_t>();
    std::vector<idx_t> xadj = dcrs.offsets_as<idx_t>();
    std::vector<idx_t> adjncy = dcrs.indices_as<idx_t>();

    // Actual call to ParMETIS.
    int result = ParMETIS_V3_PartKway(&vtxdist[0], &xadj[0], &adjncy[0],
      nullptr, nullptr, &wgtflag, &numflag, &ncon, &size, &tpwgts[0], ubvec.data(),
      options, &edgecut, &part[0], &comm);
    if (result != METIS_OK) clog_error( "Parmetis failed!" );

    std::vector<size_t> partitioning( part.begin(), part.end() );

    return partitioning;

  } // color

}; // struct parmetis_colorer_t

} // namespace coloring
} // namespace flecsi
