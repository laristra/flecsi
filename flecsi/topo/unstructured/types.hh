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
#endif

#include <cstddef>
#include <set>

namespace flecsi {
namespace topo {

namespace unstructured_impl {

struct shared_entity {
  size_t id;
  std::vector<size_t> dependants;
};

struct ghost_entity {
  size_t id;
  size_t color;
};

struct index_coloring {
  std::set<size_t> primary;
  std::set<size_t> exclusive;
  std::set<shared_entity> shared;
  std::set<ghost_entity> ghost;
};

struct coloring_meta {
  size_t exclusive;
  size_t shared;
  size_t ghost;

  std::set<size_t> dependants; // depend on us
  std::set<size_t> dependencies; // we depend on them
};

/*
  Closure tokens for specifying the behavior of closure function.
 */

template<size_t IndexSpace,
  size_t Dimension,
  size_t ThroughDimension,
  size_t Depth = 1>
struct primary_independent {
  static constexpr size_t index_space = IndexSpace;
  static constexpr size_t dimension = Dimension;
  static constexpr size_t thru_dimension = ThroughDimension;
  static constexpr size_t depth = Depth;
}; // struct primary_independent

template<size_t IndexSpace, size_t Dimension, size_t PrimaryDimension>
struct auxiliary_independent {
  static constexpr size_t index_space = IndexSpace;
  static constexpr size_t dimension = Dimension;
  static constexpr size_t primary_dimension = PrimaryDimension;
}; // struct auxiliary_independent

} // namespace unstructured_impl

struct unstructured_base {

  using index_coloring = unstructured_impl::index_coloring;
  using shared_entity = unstructured_impl::shared_entity;
  using ghost_entity = unstructured_impl::ghost_entity;
  using coloring_meta = unstructured_impl::coloring_meta;

  struct coloring {
    std::vector<index_coloring> indices;
    std::vector<std::vector<coloring_meta>> distribution;
  }; // struct coloring

}; // struct unstructured_base

} // namespace topo
} // namespace flecsi
