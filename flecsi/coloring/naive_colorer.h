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

#include <flecsi/coloring/colorer.h>
#include <flecsi/utils/mpi_type_traits.h>

namespace flecsi {
namespace coloring {

/*!
  The colorer_t type provides a ParMETIS implementation of the
  colorer_t interface.
 */

struct naive_colorer_t : public colorer_t {
  /*!
   Default constructor
   */
  naive_colorer_t() {}

  /*!
   Copy constructor (disabled)
   */
  naive_colorer_t(const naive_colorer_t &) = delete;

  /*!
   Assignment operator (disabled)
   */
  naive_colorer_t & operator=(const naive_colorer_t &) = delete;

  /*!
    Destructor
   */
  ~naive_colorer_t() {}

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
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    auto start = dcrs.distribution[world_rank];
    auto my_ents = dcrs.distribution[world_rank+1] - start;
    auto num_ents = dcrs.distribution[world_size];

    auto r = num_ents / num_parts;
    auto q = num_ents % num_parts;

    std::vector<size_t> newdist(num_parts+1);
    newdist[0] = 0;

    for (int i=0; i<num_parts; ++i) {
      size_t n = (i<q) ? r+1 : r;
      newdist[i+1] = newdist[i] + n;
    }

    std::vector<size_t> part(my_ents);
    for (size_t i=0, r=0; i<my_ents; ++i) {
      auto global_id = start + i;
      while (global_id>=newdist[r+1]) {r++;}
      part[i] = r;
    }

    return part;

  } // color

}; // struct naive_colorer_t

} // namespace coloring
} // namespace flecsi
