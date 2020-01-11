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
#include <unordered_map>

namespace flecsi {
namespace topology {
namespace unstructured_impl {

/*!
  FIXME Add description.
 */
struct index_coloring {
  // using entity_info = flecsi::topology::unstructured_impl::entity_info;

  //------------------------------------------------------------------------//
  // Data members.
  //------------------------------------------------------------------------//

  // Set of mesh ids of the primary coloring
  std::set<size_t> primary;

  // Set of entity_info type of the exclusive coloring
  std::set<entity_info> exclusive;

  // Set of entity_info type of the shared coloring
  std::set<entity_info> shared;

  // Set of entity_info type of the ghost coloring
  std::set<entity_info> ghost;

  // Rank id to number of entities
  std::unordered_map<size_t, size_t> entities_per_rank;

  /*!
    Equality operator.

    \param ip The index_coloring to compare with \e this.

    \return True if \e ip is equivalent to \e this, false otherwise.
   */

  bool operator==(const index_coloring & ip) const {
    return (this->primary == ip.primary && this->exclusive == ip.exclusive &&
            this->shared == ip.shared && this->ghost == ip.ghost);
  } // operator ==

}; // struct index_coloring

} // namespace unstructured_impl
} // namespace topology
} // namespace flecsi
