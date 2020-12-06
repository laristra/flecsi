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
  std::size_t id;
  std::vector<std::size_t> dependents;
};

struct ghost_entity {
  std::size_t id;
  std::size_t color;
};

struct index_coloring {
  std::vector<std::size_t> exclusive;
  std::vector<shared_entity> shared;
  std::vector<ghost_entity> ghost;
};

// FIXME: may not need this
struct crs {
  std::vector<std::size_t> offsets;
  std::vector<std::size_t> indices;
};

struct coloring {
  std::size_t colors;
  std::vector<std::size_t> idx_allocs;
  std::vector<index_coloring> idx_colorings; // not sure about this
  std::vector<std::vector<std::size_t>> cnx_allocs;
  std::vector<std::vector<crs>> cnx_colorings; // not sure about this
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
  using crs = unstructured_impl::crs;

  struct coloring {
    std::size_t colors;
    std::vector<std::size_t> idx_allocs;
    std::vector<index_coloring> idx_colorings;
    std::vector<std::vector<std::size_t>> cnx_allocs;
    std::vector<std::vector<crs>> cnx_colorings;
  };

  static std::size_t idx_size(
    unstructured_impl::index_coloring const & index_coloring,
    std::size_t) {
    return index_coloring.exclusive.size() + index_coloring.shared.size() +
           index_coloring.ghost.size();
  }

  static void cnx_size(std::size_t size, resize::Field::accessor<wo> a) {
    a = data::partition::make_row(color(), size);
  }

}; // struct unstructured_base

} // namespace topo

/*----------------------------------------------------------------------------*
  Serialization Rules
 *----------------------------------------------------------------------------*/

template<>
struct util::serial<topo::unstructured_impl::shared_entity> {
  using type = topo::unstructured_impl::shared_entity;
  template<class P>
  static void put(P & p, const type & s) {
    serial_put(p, std::tie(s.id, s.dependents));
  }
  static type get(const std::byte *& p) {
    const serial_cast r{p};
    return type{r, r};
  }
};

template<>
struct util::serial<topo::unstructured_impl::index_coloring> {
  using type = topo::unstructured_impl::index_coloring;
  template<class P>
  static void put(P & p, const type & c) {
    serial_put(p, std::tie(c.exclusive, c.shared, c.ghost));
  }
  static type get(const std::byte *& p) {
    const serial_cast r{p};
    return type{r, r, r};
  }
};

} // namespace flecsi

#if 0
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

struct entity_info {
  std::size_t id;
  std::size_t rank;
  std::size_t offset;
  std::set<std::size_t> shared;

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
#endif

#if 0
struct coloring_meta {
  size_t exclusive;
  size_t shared;
  size_t ghost;

  std::set<size_t> dependents; // depend on us
  std::set<size_t> dependencies; // we depend on them
};
#endif
