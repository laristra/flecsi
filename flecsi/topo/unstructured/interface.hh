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
#include "flecsi/data/copy_plan.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/flog.hh"
#include "flecsi/topo/core.hh"
#include "flecsi/topo/unstructured/closure_utils.hh"
#include "flecsi/topo/unstructured/coloring_utils.hh"
#include "flecsi/topo/unstructured/types.hh"
#include "flecsi/topo/utility_types.hh"
#include "flecsi/util/color_map.hh"
#include "flecsi/util/dcrs.hh"
#include "flecsi/util/set_utils.hh"
#include "flecsi/util/tuple_visitor.hh"

#include <map>
#include <utility>

namespace flecsi {
namespace topo {

/*----------------------------------------------------------------------------*
  Unstructured Topology.
 *----------------------------------------------------------------------------*/

template<typename Policy>
struct unstructured : unstructured_base,
                      with_ragged<Policy>,
                      with_meta<Policy> {
  using index_space = typename Policy::index_space;
  using index_spaces = typename Policy::index_spaces;

  template<std::size_t>
  struct access;

  unstructured(coloring const & c)
    : with_ragged<Policy>(c.colors), with_meta<Policy>(c.colors),
      part_(make_partitions(c,
        index_spaces(),
        std::make_index_sequence<index_spaces::size>())) {
    init_ragged(index_spaces());
    allocate_connectivities(c, connect_);
  }

  static inline const connect_t<Policy> connect_;
  static inline const lists_t<Policy> special_;
  util::key_array<repartitioned, index_spaces> part_;

  std::size_t colors() const {
    return part_.front().colors();
  }

  template<index_space S>
  data::region & get_region() {
    return part_.template get<S>();
  }
  template<index_space S>
  const data::partition & get_partition(field_id_t) const {
    return part_.template get<S>();
  }

private:
  template<auto... Value, std::size_t... Index>
  util::key_array<repartitioned, util::constants<Value...>> make_partitions(
    unstructured_base::coloring const & c,
    util::constants<Value...> /* index spaces to deduce pack */,
    std::index_sequence<Index...>) {
    flog_assert(c.idx_colorings.size() == sizeof...(Value),
      c.idx_colorings.size()
        << " sizes for " << sizeof...(Value) << " index spaces");
    return {{make_repartitioned<Policy, Value>(
      c.colors, make_partial<idx_size>(c.idx_colorings[Index]))...}};
  }

  template<auto... VV, typename... TT>
  void allocate_connectivities(const unstructured_base::coloring & c,
    util::key_tuple<util::key_type<VV, TT>...> const & /* deduce pack */) {
    std::size_t entity = 0;
    (
      [&](TT const & row) {
        auto & cc = c.cnx_allocs[entity++];
        std::size_t is{0};
        for(auto & fd : row) {
          auto & p = this->ragged.template get_partition<VV>(fd.fid);
          execute<cnx_size>(cc[is++], p.sizes());
        }
      }(connect_.template get<VV>()),
      ...);
  }

  template<index_space... SS>
  void init_ragged(util::constants<SS...>) {
    (this->template extend_offsets<SS>(), ...);
  }
}; // struct unstructured

/*----------------------------------------------------------------------------*
  Unstructured Access.
 *----------------------------------------------------------------------------*/

template<typename Policy>
template<std::size_t Privileges>
struct unstructured<Policy>::access {
private:
  using entity_list = typename Policy::entity_list;
  template<const auto & Field>
  using accessor = data::accessor_member<Field, Privileges>;
  util::key_array<resize::accessor<ro>, index_spaces> size_;
  connect_access<Policy, Privileges> connect_;
  list_access<Policy, Privileges> special_{unstructured::special_};

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

  template<index_space I, entity_list L>
  auto special_entities() const {
    return make_ids<I>(special_.template get<I>().template get<L>()[0]);
  }

  template<class F>
  void send(F && f) {
    std::size_t i = 0;
    for(auto & a : size_)
      f(a, [&i](typename Policy::slot & u) { return u->part_[i++].sizes(); });
    connect_send(f, connect_, unstructured::connect_);
    connect_send(f, special_, unstructured::special_, &unstructured::meta);
  }
}; // struct unstructured<Policy>::access

template<>
struct detail::base<unstructured> {
  using type = unstructured_base;
}; // struct detail::base<unstructured>

} // namespace topo
} // namespace flecsi
