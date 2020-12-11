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

struct parmetis_repart_colorer_t : public colorer_t {
  /*!
   Default constructor
   */
  parmetis_repart_colorer_t() = default;

  /*!
   Copy constructor (disabled)
   */
  parmetis_repart_colorer_t(const parmetis_repart_colorer_t &) = delete;

  /*!
   Assignment operator (disabled)
   */
  parmetis_repart_colorer_t & operator=(const parmetis_repart_colorer_t &) = delete;

  /*!
    Destructor
   */
  ~parmetis_repart_colorer_t() {}

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

  std::vector<size_t> new_color(size_t num_parts, const dcrs_t & dcrs) override {
  
    auto num_ents = dcrs.size();
    auto comm = create_communicator(MPI_COMM_WORLD, num_ents);
  
    if (num_ents) {

      idx_t wgtflag = 0;
      idx_t numflag = 0;
      idx_t ncon = 1;
      std::vector<real_t> tpwgts(ncon * num_parts, 1.0 / num_parts);
  
      // We may need to expose some of the ParMETIS configuration options.
      std::vector<real_t> ubvec(ncon, 1.05);
      idx_t options[3] = {0, 0, 0};
      idx_t edgecut;

      // Get the dCRS information using ParMETIS types.
      auto xadj = dcrs.offsets_as<idx_t>();
      auto adjncy = dcrs.indices_as<idx_t>();
      
      // colapse distribution
      int rank, size, world_size, world_rank;
      MPI_Comm_rank(comm, &rank);
      MPI_Comm_size(comm, &size);
      MPI_Comm_size(MPI_COMM_WORLD, &world_size);
      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

      if (rank==0) {
        if (num_parts != size) {
          std::cout << "WARNING: repartitiioner will not create new partitions" << std::endl;
        }
      }

      std::vector<idx_t> vtxdist(size+1);
      vtxdist[0] = 0;
      for (int r=0, i=0; r<world_size; ++r) {
        auto n = dcrs.distribution[r+1] - dcrs.distribution[r];
        if (n>0) {
          vtxdist[i+1] = vtxdist[i] + n;
          i++;
        }
      }
      
      int defval = (size==num_parts) ? std::numeric_limits<idx_t>::max() : world_rank;
      std::vector<idx_t> part(dcrs.size(), defval);
      
      // Actual call to ParMETIS.
      idx_t npart = num_parts;
      int result = ParMETIS_V3_RefineKway(&vtxdist[0], &xadj[0], &adjncy[0],
        nullptr, nullptr, &wgtflag, &numflag, &ncon, &npart, &tpwgts[0],
        ubvec.data(), options, &edgecut, &part[0], &comm);
      if(result != METIS_OK)
        clog_error("Parmetis failed!");
  
      std::vector<size_t> partitioning(part.begin(), part.end());
      std::set<size_t> un(part.begin(), part.end());

      return partitioning;
    }
    else {
      return {};
    }

  } // color

}; // struct parmetis_repart_colorer_t

} // namespace coloring
} // namespace flecsi
