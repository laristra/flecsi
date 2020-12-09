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
#include "flecsi/data/topology_slot.hh"
#include "flecsi/execution.hh"
#include "flecsi/flog.hh"
#include "flecsi/topo/canonical/types.hh"
#include "flecsi/topo/core.hh" // base
#include "flecsi/topo/utility_types.hh"

#include <string>

namespace flecsi {
namespace topo {

/*!
  The canonical type is a dummy topology for development and testing.

  @ingroup topology
 */

template<typename Policy>
struct canonical : canonical_base, with_ragged<Policy> {
  using index_space = typename Policy::index_space;
  using index_spaces = typename Policy::index_spaces;
  using connectivities = typename Policy::connectivities;

  template<std::size_t>
  struct access;

  canonical(const coloring & c)
    : with_ragged<Policy>(c.colors),
      part_(make_partitions(c,
        index_spaces(),
        std::make_index_sequence<index_spaces::size>())),
      meta_(c.colors) {
    init_ragged(index_spaces());
    allocate_connectivities(c, connect_);
  }

  // The first index space is distinguished in that we decorate it:
  static inline const field<int>::definition<Policy, index_spaces::first> mine_;
  static inline const connect_t<Policy> connect_;

  util::key_array<repartitioned, index_spaces> part_;
  meta_topo::core meta_;

  // These functions are part of the standard topology interface.
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
  template<auto... VV, std::size_t... II>
  util::key_array<repartitioned, util::constants<VV...>> make_partitions(
    const canonical_base::coloring & c,
    util::constants<VV...> /* index_spaces, to deduce a pack */,
    std::index_sequence<II...>) {
    flog_assert(c.is_allocs.size() == sizeof...(VV),
      c.is_allocs.size() << " sizes for " << sizeof...(VV) << " index spaces");
    return {{make_repartitioned<Policy, VV>(c.colors,
      make_partial<is_size>(c.is_allocs[II], c.is_allocs.size()))...}};
  }

  template<auto... VV, typename... TT>
  void allocate_connectivities(const canonical_base::coloring & c,
    util::key_tuple<util::key_type<VV, TT>...> const & /* deduce pack */) {
    std::size_t entity = 0;
    (
      [&](TT const & row) {
        auto & cc = c.cn_allocs[entity++];
        std::size_t is{0};
        for(auto & fd : row) {
          auto & p = this->ragged.template get_partition<VV>(fd.fid);
          execute<cn_size>(cc[is++], p.sizes());
        }
      }(connect_.template get<VV>()),
      ...);
  }

  template<index_space... SS>
  void init_ragged(util::constants<SS...>) {
    (this->template extend_offsets<SS>(), ...);
  }

}; // struct canonical

template<class P>
template<std::size_t Priv>
struct canonical<P>::access {
private:
  template<const auto & F>
  using accessor = data::accessor_member<F, Priv>;
  util::key_array<resize::accessor<ro>, index_spaces> size;
  connect_access<P, Priv> connect_;

public:
  accessor<canonical::mine_> mine_;
  accessor<meta_field> meta_;

  access() : connect_(canonical::connect_) {}

  // NB: iota_view's iterators are allowed to outlive it.
  template<index_space S>
  auto entities() {
    return make_ids<S>(util::iota_view<util::id>(
      0, data::partition::row_size(size.template get<S>())));
  }

  template<index_space F, index_space T>
  auto & get_connect() {
    return connect_.template get<F>().template get<T>();
  }
  template<index_space F, index_space T>
  const auto & get_connect() const {
    return connect_.template get<F>().template get<T>();
  }

  template<index_space T, index_space F>
  auto entities(id<F> i) const {
    return make_ids<T>(get_connect<F, T>()[i]);
  }

  template<class F>
  void send(F && f) {
    std::size_t i = 0;
    for(auto & a : size)
      f(a, [&i](typename P::slot & c) { return c->part_[i++].sizes(); });
    mine_.topology_send(f);
    meta_.topology_send(f, &canonical::meta_);
    connect_send(f, connect_, canonical::connect_);
  }
};

template<>
struct detail::base<canonical> {
  using type = canonical_base;
};

} // namespace topo
} // namespace flecsi
