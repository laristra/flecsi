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

#include "flecsi/execution.hh"
#include "flecsi/topo/index.hh"
#include "flecsi/util/serialize.hh"

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

struct entity_info {
  size_t id;
  size_t rank;
  size_t offset;
  std::set<size_t> shared;

  /*!
   Constructor.

   \param id_     The entity id. This is generally specified by the
                  mesh definition.
   \param rank_   The rank that owns this entity.
   \param offset_ The local id or offset of the entity.
   \param shared_ The list of ranks that share this entity.
   */

  entity_info(size_t id_ = 0,
    size_t rank_ = 0,
    size_t offset_ = 0,
    std::set<size_t> shared_ = {})
    : id(id_), rank(rank_), offset(offset_), shared(shared_) {}

  /*!
   Constructor.

   \param id_     The entity id. This is generally specified by the
                  mesh definition.
   \param rank_   The rank that owns this entity.
   \param offset_ The local id or offset of the entity.
   \param shared_ The rank that shares this entity.
   */

  entity_info(size_t id_, size_t rank_, size_t offset_, size_t shared_)
    : id(id_), rank(rank_), offset(offset_) {
    shared.emplace(shared_);
  }

  entity_info(size_t id_,
    size_t rank_,
    size_t offset_,
    std::vector<size_t> shared_)
    : id(id_), rank(rank_), offset(offset_),
      shared(shared_.begin(), shared_.end()) {}

  /*!
   Comparison operator for container insertion. This sorts by the
   entity id, e.g., as set by the id_ parameter to the constructor.
   */

  bool operator<(const entity_info & c) const {
    return id < c.id;
  } // operator <

  /*!
    Comparison operator for equivalence.
   */

  bool operator==(const entity_info & c) const {
    return id == c.id && rank == c.rank && offset == c.offset &&
           shared == c.shared;
  } // operator ==

}; // struct entity_info

struct index_coloring {
  std::set<size_t> primary;
  std::set<entity_info> exclusive;
  std::set<entity_info> shared;
  std::set<entity_info> ghost;
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
  using entity_info = unstructured_impl::entity_info;
  using coloring_meta = unstructured_impl::coloring_meta;

  struct coloring {
    std::size_t colors;
    std::vector<index_coloring> index_colorings;
    std::vector<std::vector<std::size_t>> connectivity_sizes;
    std::vector<std::vector<coloring_meta>> distribution;
  }; // struct coloring

  static std::size_t is_size(
    unstructured_impl::index_coloring const & index_coloring,
    std::size_t colors,
    std::size_t color) {
    (void)index_coloring;
    (void)colors;
    (void)color;
    return 10;
  }

  static void cn_size(std::size_t size, resize::Field::accessor<wo> a) {
    a = data::partition::make_row(color(), size);
  }

}; // struct unstructured_base

} // namespace topo

/*----------------------------------------------------------------------------*
  Serialization Rules
 *----------------------------------------------------------------------------*/

template<>
struct util::serial<topo::unstructured_impl::entity_info> {
  using type = topo::unstructured_impl::entity_info;
  template<class P>
  static void put(P & p, const type & c) {
    serial_put(p, std::tie(c.id, c.rank, c.offset, c.shared));
  }
  static type get(const std::byte *& p) {
    const serial_cast r{p};
    return type{r, r, r, std::set<std::size_t>(r)};
  }
};

template<>
struct util::serial<topo::unstructured_impl::index_coloring> {
  using type = topo::unstructured_impl::index_coloring;
  template<class P>
  static void put(P & p, const type & c) {
    serial_put(p, std::tie(c.primary, c.exclusive, c.shared, c.ghost));
  }
  static type get(const std::byte *& p) {
    const serial_cast r{p};
    return type{r, r, r, r};
  }
};

} // namespace flecsi
