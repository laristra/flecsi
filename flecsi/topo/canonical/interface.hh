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
struct canonical : canonical_base {
  using index_space = typename Policy::index_space;
  using index_spaces = typename Policy::index_spaces;

  template<std::size_t>
  struct access;

  template<class F>
  static void fields(F f) {
    f(mine);
    connect_visit(f, connect);
  }

  canonical(const coloring & c)
    : part(make_partitions(c,
        index_spaces(),
        std::make_index_sequence<index_spaces::size>())) {}

  // The first index space is distinguished in that we decorate it:
  static inline const field<int>::definition<Policy, index_spaces::first> mine;
  static inline const connect_t<Policy> connect;

  util::key_array<repartitioned, index_spaces> part;

  std::size_t colors() const {
    return part.front().colors();
  }

  template<index_space S>
  data::region & get_region() {
    return part.template get<S>();
  }
  template<index_space S>
  const data::partition & get_partition() const {
    return part.template get<S>();
  }

private:
  template<auto... VV, std::size_t... II>
  util::key_array<repartitioned, util::constants<VV...>> make_partitions(
    const canonical_base::coloring & c,
    util::constants<VV...> /* index_spaces, to deduce a pack */,
    std::index_sequence<II...>) {
    flog_assert(c.sizes.size() == sizeof...(VV),
      c.sizes.size() << " sizes for " << sizeof...(VV) << " index spaces");
    return {{make_repartitioned<Policy, VV>(
      c.parts, make_partial<allocate>(c.sizes[II], c.parts))...}};
  }
}; // struct canonical

template<class P>
template<std::size_t Priv>
struct canonical<P>::access {
  template<const auto & F>
  using accessor = data::accessor_member<F, Priv>;
  accessor<canonical::mine> mine;
  connect_access<P, Priv> connect;

  access() : connect(canonical::connect) {}

  template<index_space F, index_space T>
  auto & get_connect() {
    return connect.template get<F>().template get<T>();
  }
  template<index_space F, index_space T>
  const auto & get_connect() const {
    return connect.template get<F>().template get<T>();
  }

  template<class F>
  void bind(F f) {
    f(mine);
    connect_visit(f, connect);
  }
};

template<>
struct detail::base<canonical> {
  using type = canonical_base;
};

} // namespace topo
} // namespace flecsi
