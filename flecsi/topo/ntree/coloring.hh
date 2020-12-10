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

    coloring(std::size_t nparts)
      : nparts_(nparts), global_hmap_(nparts * local_hmap_),
        hmap_offset_(nparts, local_hmap_), tdata_offset_(nparts, 3) {}

    // Global
    size_t nparts_;

    // Entities
    size_t local_entities_;
    size_t global_entities_;
    std::vector<std::size_t> entities_distribution_;
    std::vector<std::size_t> entities_offset_;

    // nodes
    size_t local_nodes_;
    size_t global_nodes_;
    std::vector<std::size_t> nodes_offset_;

    // hmap
    static constexpr size_t local_hmap_ = 1 << 15;
    size_t global_hmap_;
    std::vector<std::size_t> hmap_offset_;

    // tdata
    std::vector<std::size_t> tdata_offset_;

    // All global sizes array for make_partition
    std::vector<std::size_t> global_sizes_;
  }; // struct coloring

  static std::size_t allocate(const std::vector<std::size_t> & arr,
    const std::size_t & i) {
    return arr[i];
  }

  static void set_dests(field<data::intervals::Value>::accessor<wo> a) {
    assert(a.span().size() == 1);
    a[0] = data::intervals::make({1, 3});
  }
  static void set_ptrs(field<data::points::Value>::accessor<wo> a) {
    const auto & c = run::context::instance();
    const auto i = c.color(), n = c.colors();
    assert(a.span().size() == 3);
    a[1] = data::points::make(i == 0 ? i : i - 1, 0);
    a[2] = data::points::make(i == n - 1 ? i : i + 1, 0);
  }
  template<auto * F> // work around Clang 10.0.1 bug with auto&
  static constexpr inline auto task = [](auto f) { execute<*F>(f); };
}; // struct ntree_base

} // namespace topo
} // namespace flecsi
