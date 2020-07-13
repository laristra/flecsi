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
namespace topo {

//----------------------------------------------------------------------------//
// NTree topology coloring.
//----------------------------------------------------------------------------//

struct ntree_base {
  enum index_space { entities };
  //static constexpr std::size_t index_spaces = 1;
  using index_spaces = util::constants<entities>;

  struct coloring {
    size_t local_entities_;
    size_t global_entities_; 
    std::vector<size_t> entities_distribution_;
    size_t nparts_; 
    std::vector<std::pair<size_t,size_t>> offset_;  
  }; // struct coloring
}; // struct ntree_base

// using coloring_t = std::map<size_t, index_coloring_t>;

} // namespace topo
} // namespace flecsi
