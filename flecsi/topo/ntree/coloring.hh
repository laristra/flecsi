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
  enum index_space { entities, nodes, hashmap, tree_data };
  // static constexpr std::size_t index_spaces = 1;
  using index_spaces = util::constants<entities, nodes, hashmap, tree_data>;

  struct coloring {
    // Global
    size_t processes_;

    // Entities 
    size_t local_entities_;
    size_t global_entities_;
    std::vector<size_t> entities_distribution_;
    std::vector<std::pair<size_t,size_t>> entities_offset_;

    //nodes
    size_t local_nodes_; 
    size_t global_nodes_; 
    std::vector<std::pair<size_t,size_t>> nodes_offset_; 

    // hmap 
    const size_t local_hmap_ = 1<<15;  
    size_t global_hmap_; 
    std::vector<std::pair<size_t,size_t>> hmap_offset_; 

    // tdata 
    std::vector<std::pair<size_t,size_t>> tdata_offset_; 

    // All global sizes array for make_partition
    std::vector<size_t> global_sizes_; 
  }; // struct coloring
}; // struct ntree_base

} // namespace topo
} // namespace flecsi
