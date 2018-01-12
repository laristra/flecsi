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

#include <cassert>
#include <unordered_map>
#include <vector>

#include <flecsi/coloring/communicator.h>

namespace flecsi {
namespace coloring {

/*!
  FIXME Add description.
 */
struct index_coloring_t {
  using entity_info_t = flecsi::coloring::entity_info_t;

  //------------------------------------------------------------------------//
  // Data members.
  //------------------------------------------------------------------------//

  // Set of mesh ids of the primary coloring
  std::set<size_t> primary;

  // Set of entity_info_t type of the exclusive coloring
  std::set<entity_info_t> exclusive;

  // Set of entity_info_t type of the shared coloring
  std::set<entity_info_t> shared;

  // Set of entity_info_t type of the ghost coloring
  std::set<entity_info_t> ghost;

  // Rank id to number of entities
  std::unordered_map<size_t, size_t> entities_per_rank;

  /*!
   Equality operator.
  
   \param ip The index_coloring_t to compare with \e this.
  
   \return True if \e ip is equivalent to \e this, false otherwise.
   */
  bool operator==(const index_coloring_t & ip) const {
    return (
        this->primary == ip.primary && this->exclusive == ip.exclusive &&
        this->shared == ip.shared && this->ghost == ip.ghost);
  } // operator ==

}; // struct index_coloring_t

} // namespace coloring
} // namespace flecsi
