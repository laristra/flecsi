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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <map>
#include <vector>

namespace flecsi {
namespace coloring {

/*!
  Local coloring information for a topological index space.
 */

struct index_coloring_t {

  /*!
    Type for passing information about shared entities.
   */

  struct shared_info_t {
    size_t global_id;
    size_t color;
    size_t local_id;
    std::vector<size_t> used_by_colors;
  }; // struct shared_info_t

  /*!
    Type for passing information about ghost entities.
   */

  struct ghost_info_t {
    size_t global_id;
    size_t owned_by_color;
  }; // struct ghost_info_t

  /*!
    Set of mesh ids of the primary (disjoint) coloring. In general, this is the
    union of exclusive and shared.
   */

  std::vector<size_t> primary;

  /*!
    Set of exclusive indices.
   */

  std::vector<size_t> exclusive;

  /*!
    Set of shared indices.
   */

  std::vector<shared_info_t> shared;

  /*!
    Set of ghost indices.
   */

  std::vector<ghost_info_t> ghost;

}; // struct index_coloring_t

/*!
  Aggregate coloring information for a topological index space.
 */

struct coloring_info_t {
  size_t exclusive;
  size_t shared;
  size_t ghost;

  std::vector<size_t> used_by_colors;
  std::vector<size_t> owned_by_colors;
}; // struct coloring_info_t

/*!
  Define the coloring type.

  The map key is the index space identifier.
 */

using coloring_t = std::map<size_t, index_coloring_t>;

/*!
  Type for holding aggregated (from all colors) coloring info.

  The map key is the color and index space.
 */

using coloring_meta_t = std::map<std::pair<size_t, size_t>, coloring_info_t>;

} // namespace coloring
} // namespace flecsi
