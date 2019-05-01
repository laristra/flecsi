/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/utils/flog.h>
#endif

#include <unordered_map>

namespace flecsi {
namespace execution {

template<typename TOPOLOGY_TYPE>
struct topology_instance_u {

  //--------------------------------------------------------------------------//
  // Coloring interface
  //--------------------------------------------------------------------------//

  using coloring_t = typename TOPOLOGY_TYPE::coloring_t;

  template<size_t IDENTIFIER>
  void add_coloring(coloring_t const & coloring) {

    flog_assert(coloring_map_.find(IDENTIFIER) == coloring_map_.end(),
      "coloring already exists");

    coloring_map_[IDENTIFIER] = coloring;
  } // add_coloring

  template<size_t IDENTIFIER>
  void remove_coloring() {
    auto ita = coloring_map_.find(IDENTIFIER);
    flog_assert(ita != coloring_map_.end(), "launch map does not exist");

    coloring_map_.erase(ita);
  } // remove_coloring

  //--------------------------------------------------------------------------//
  // Launch mapping interface
  //--------------------------------------------------------------------------//

  /*!
    Add a launch map. Conceptually, a launch map is a mapping between a color
    and a task index. The map is implemented as a std::vector<size_t>, where
    the size of the vector is the launch domain size, each vector index
    corresponds to the task index, and the vector value at that index
    corresponds to the color that should be mapped to that task index.

    @tparam IDENTIFIER A hash that identifies the launch_map.

    @param launch_map A std::vector<size_t> that defines the mapping of colors
                      to task indices.
   */

  template<size_t IDENTIFIER>
  void add_launch_map(std::vector<size_t> const & launch_map) {

    flog_assert(launch_map_.find(IDENTIFIER) == launch_map_.end(),
      "launch map already exists");

    launch_map_[IDENTIFIER] = launch_map;
  } // add_launch_map

  /*!
    Remove a launch map.

    @tparam IDENTIFIER A hash that identifies the launch_map.
   */

  template<size_t IDENTIFIER>
  void remove_launch_map() {
    auto ita = launch_map_.find(IDENTIFIER);
    flog_assert(ita != launch_map_.end(), "launch map does not exist");

    launch_map_.erase(ita);
  } // remove_launch_map

private:
  std::unordered_map<size_t, coloring_t> coloring_map_;
  std::unordered_map<size_t, std::vector<size_t>> launch_map_;

}; // struct topology_instance_u

} // namespace execution
} // namespace flecsi
