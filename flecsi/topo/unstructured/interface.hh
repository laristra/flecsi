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

#include "flecsi/data/accessor.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/flog.hh"
#include "flecsi/topo/core.hh"
#include "flecsi/topo/unstructured/types.hh"
#include "flecsi/topo/utility_types.hh"
#include "flecsi/util/color_map.hh"
#include "flecsi/util/dcrs.hh"

#include <map>
#include <utility>

namespace flecsi {
namespace topo {

namespace unstructured_impl {

/*----------------------------------------------------------------------------*
  Method Implementations.
 *----------------------------------------------------------------------------*/

/*
  This is a test implmentation to be used in developing and testing the
  unstructured topology type. It is not scalable, and should not be used
  for real problems!
 */

template<typename Definition>
inline util::dcrs
naive_coloring(Definition const & md,
  size_t entity_dimension,
  size_t thru_dimension,
  size_t process = run::context::instance().process(),
  size_t processes = run::context::instance().processes()) {

  /*
    This utility isn't really necessary here (since we are using colors =
    processes, but it is convenient nonetheless.
   */
  util::color_map cm(processes, processes, md.num_entities(entity_dimension));

  flog_assert(entity_dimension != thru_dimension,
    "thru dimension cannot equal entity dimension");

  /*
    The current implementation only supports graph creation of
    the highest dimensional entity type through lower dimensional types,
    e.g., cells through vertices, or edges.
    Support could be added for the inverse, e.g., vertices through edges,
    or cells. This assertion catches the unsupported case.
   */
  flog_assert(entity_dimension > thru_dimension,
    "current implementation does not support through dimensions"
    " greater than entity dimension");

  std::map<size_t, std::vector<size_t>> thru2entity;

  // This is not scalable!
  for(size_t e{0}; e < md.num_entities(entity_dimension); ++e) {
    for(auto v : md.entities(entity_dimension, 0, e)) {
      thru2entity[v].push_back(e);
    } // for
  } // for

  util::dcrs dcrs;
  dcrs.distribution = cm.distribution();

  const size_t color_offset = cm.index_offset(process, 0);
  const size_t color_indices = cm.indices(process, 0);

  std::map<size_t, std::vector<size_t>> entity2entities;

  for(size_t e{color_offset}; e < color_offset + color_indices; ++e) {
    std::map<size_t, size_t> thru_counts;

    for(auto v : md.entities(entity_dimension, 0, e)) {
      for(auto o : thru2entity[v]) {
        if(o != e) {
          thru_counts[o] += 1;
        } // if
      } // for
    } // for

    for(auto tc : thru_counts) {
      if(tc.second > thru_dimension) {
        entity2entities[e].push_back(tc.first);
        entity2entities[tc.first].push_back(e);
      } // if
    } // for
  } // for

  dcrs.offsets.push_back(0);
  for(size_t i{0}; i < color_indices; ++i) {
    auto e = dcrs.distribution[process] + i;

    std::set<size_t> set_indices;
    for(auto n : entity2entities[e]) {
      set_indices.insert(n);
    } // for

    for(auto i : set_indices) {
      dcrs.indices.push_back(i);
    } // for

    dcrs.offsets.push_back(dcrs.offsets[i] + set_indices.size());
  } // for

  flog_devel(info) << dcrs << std::endl;

  return dcrs;
} // naive_coloring

template<typename Policy>
unstructured_base::coloring
closure(typename Policy::definition const & md,
  std::vector<std::vector<size_t>> const & primary) {
  (void)md;
  (void)primary;
  return {};
} // closure

} // namespace unstructured_impl

/*----------------------------------------------------------------------------*
  Unstructured Topology.
 *----------------------------------------------------------------------------*/

template<typename Policy>
struct unstructured : unstructured_base, with_ragged<Policy> {
  using index_space = typename Policy::index_space;
  using index_spaces = typename Policy::index_spaces;

  template<std::size_t>
  struct access;

  unstructured(coloring const & c)
    : with_ragged<Policy>(c.colors),
      part(make_partitions(c,
        index_spaces(),
        std::make_index_sequence<index_spaces::size>())) {}

  template<typename Definition>
  inline util::dcrs naive_coloring(Definition const & md,
    size_t entity_dimension,
    size_t thru_dimension,
    size_t process = run::context::instance().process(),
    size_t processes = run::context::instance().processes()) {
    return unstructured_impl::naive_coloring(
      md, entity_dimension, thru_dimension, process, processes);
  }

  static inline const connect_t<Policy> connect_;
  util::key_array<repartitioned, index_spaces> part;

private:
  template<auto... Value, std::size_t... Index>
  util::key_array<repartitioned, util::constants<Value...>> make_partition(
    unstructured_base::coloring const & c,
    util::constants<Value...> /* index spaces to deduce pack */,
    std::index_sequence<Index...>) {
    return {{make_repartitioned<Policy, Value>(c.colors,
      make_partial<allocate>(c.index_colorings[Index], c.colors))...}};
  }
}; // struct unstructured

/*----------------------------------------------------------------------------*
  Unstructured Access.
 *----------------------------------------------------------------------------*/

template<typename Policy>
template<std::size_t Privileges>
struct unstructured<Policy>::access {
private:
  template<const auto & Field>
  using accessor = data::accessor_member<Field, Privileges>;
  util::key_array<resize::accessor, index_spaces> size_;
  connect_access<Policy, Privileges> connect_;

  template<index_space From, index_space To>
  auto & connectivity() {
    return connect_.template get<From>().template get<To>();
  }

  template<index_space From, index_space To>
  auto const & connectivity() const {
    return connect_.template get<From>().template get<To>();
  }

public:
  access() : connect_(unstructured::connect_) {}

  /*!
    Return an iterator to the parameterized index space.

    @tparam IndexSpace The index space identifier.
   */

  template<index_space IndexSpace>
  auto entities() {
    return make_ids<IndexSpace>(util::iota_view<util::id>(
      0, data::partition::row_size(size_.template get<IndexSpace>())));
  }

  /*!
    Return an iterator to the connectivity information for the parameterized
    index spaces.

    @tparam To   The connected index space.
    @tparam From The index space with connections.
   */

  template<index_space To, index_space From>
  auto entities(id<From> from) const {
    return make_ids<To>(connectivity<From, To>()[from]);
  }
}; // struct unstructured<Policy>::access

template<>
struct detail::base<unstructured> {
  using type = unstructured_base;
}; // struct detail::base<unstructured>

} // namespace topo
} // namespace flecsi
