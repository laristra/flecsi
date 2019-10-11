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
namespace topology {

//----------------------------------------------------------------------------//
// NTree topology coloring.
//----------------------------------------------------------------------------//

struct ntree_topology_base {

  struct coloring {

    struct local_coloring {
      size_t local_entities_;
    }; // local_coloring

    struct coloring_metadata {
      std::vector<size_t> entities_distribution_;
    }; // coloring_metadata

    void init() {}

    local_coloring local_coloring_;
    coloring_metadata coloring_metadata_;

  }; // struct coloring
}; // struct ntree_topology_base

// using coloring_t = std::map<size_t, index_coloring_t>;

} // namespace topology
} // namespace flecsi
