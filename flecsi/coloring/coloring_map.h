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
#else
#include <flecsi/utils/flog.h>
#endif

#include <unordered_map>

namespace flecsi {
namespace coloring {

template<typename TOPOLOGY_TYPE>
struct coloring_map_u {

  using coloring_t = typename TOPOLOGY_TYPE::coloring_t;

  struct coloring_map_t : public std::unordered_map<size_t, coloring_t> {};
  using instance_map_t = std::unordered_map<size_t, coloring_map_t>;

  bool register_coloring(size_t instance_identifier,
    size_t coloring_identifier) {

    flog_assert(instances_[instance_identifier].find(coloring_identifier) ==
                  instances_[instance_identifier].end(),
      "coloring " << coloring_identifier << " already exists");

    instances_[instance_identifier][coloring_identifier] = {};
    return true;
  } // register_coloring

  coloring_t & get_coloring(size_t instance_identifier,
    size_t coloring_identifier) {

    flog_assert(instances_.find(instance_identifier) != instances_.end(),
      "topology instance " << instance_identifier << " does not exist");

    flog_assert(instances_[instance_identifier].find(coloring_identifier) !=
                  instances_[instance_identifier].end(),
      "coloring " << coloring_identifier << " does not exist");

    return instances_[instance_identifier][coloring_identifier];
  } // colorings

private:
  instance_map_t instances_;

}; // struct coloring_map_u

#if 0
context...
using unstructured_mesh_colorings_t = coloring_map_u<...>;

template<typename TOPOLOGY_TYPE>
register_coloring(typename TOPLO... & coloring) {
  if constexpr() {
    ...
  }
#endif

} // namespace coloring
} // namespace flecsi
