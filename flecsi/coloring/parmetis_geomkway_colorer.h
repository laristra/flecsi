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

struct parmetis_geomkway_colorer_t : public colorer_t {
  /*!
   Default constructor
   */
  template<typename T>
  parmetis_geomkway_colorer_t(const std::vector<T> & xyz, idx_t ndims)
    : ndims_(ndims), xyz_(xyz.begin(), xyz.end()) {}

  /*!
   Copy constructor (disabled)
   */
  parmetis_geomkway_colorer_t(const parmetis_geomkway_colorer_t &) = delete;

  /*!
   Assignment operator (disabled)
   */
  parmetis_geomkway_colorer_t & operator=(const parmetis_geomkway_colorer_t &) = delete;

  /*!
    Destructor
   */
  ~parmetis_geomkway_colorer_t() {}

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
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
    bool has_ents = dcrs.distribution[rank+1] - dcrs.distribution[rank];
    auto comm = create_communicator(MPI_COMM_WORLD, has_ents);

    //------------------------------------------------------------------------//
    // Call ParMETIS partitioner.
    //------------------------------------------------------------------------//
    if (has_ents) {
      idx_t wgtflag = 0;
      idx_t numflag = 0;
      idx_t ncon = 1;
      std::vector<real_t> tpwgts(ncon * num_parts, 1.0 / num_parts);
  
      // We may need to expose some of the ParMETIS configuration options.
      std::vector<real_t> ubvec(ncon, 1.05);
      idx_t options[3] = {0, 0, 0};
      idx_t edgecut;
      std::vector<idx_t> part(dcrs.size(), std::numeric_limits<idx_t>::max());
  
      // Get the dCRS information using ParMETIS types.
      std::vector<idx_t> xadj = dcrs.offsets_as<idx_t>();
      std::vector<idx_t> adjncy = dcrs.indices_as<idx_t>();
      
      // colapse distribution
      int size, world_size;
      MPI_Comm_size(comm, &size);
      MPI_Comm_size(MPI_COMM_WORLD, &world_size);

      std::vector<idx_t> vtxdist(size+1);
      vtxdist[0] = 0;
      for (int r=0, i=0; r<world_size; ++r) {
        auto n = dcrs.distribution[r+1] - dcrs.distribution[r];
        if (n>0) {
          vtxdist[i+1] = vtxdist[i] + n;
          i++;
        }
      }
  
      // Actual call to ParMETIS.
      idx_t npart = num_parts;
      int result = ParMETIS_V3_PartGeomKway(&vtxdist[0], &xadj[0], &adjncy[0],
        nullptr, nullptr, &wgtflag, &numflag, &ndims_, xyz_.data(), &ncon, &npart, &tpwgts[0],
        ubvec.data(), options, &edgecut, &part[0], &comm);
      if(result != METIS_OK)
        clog_error("Parmetis failed!");
  
      std::vector<size_t> partitioning(part.begin(), part.end());
  
      return partitioning;
    }
    else {
      return {};
    }

  } // color
  
private:

  idx_t ndims_;
  std::vector<::real_t> xyz_;

}; // struct parmetis_geomkway_colorer_t

} // namespace coloring
} // namespace flecsi
