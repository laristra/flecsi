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
#include "flecsi/topo/canonical/types.hh"
#include "flecsi/topo/core.hh" // base

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
  }

  canonical(const coloring & c)
    : part(make_partitions(c,
        index_spaces(),
        std::make_index_sequence<index_spaces::size>())) {}

  // The first index space is distinguished in that we decorate it:
  static inline const field<int>::definition<Policy, index_spaces::first> mine;

  util::key_array<data::partitioned, index_spaces> part;

  std::size_t colors() const {
    return part.front().colors();
  }

  template<index_space S>
  const data::partition & get_partition() const {
    return part.template get<S>();
  }

private:
  template<auto... VV, std::size_t... II>
  static util::key_array<data::partitioned, util::constants<VV...>>
  make_partitions(const canonical_base::coloring & c,
    util::constants<VV...> /* index_spaces, to deduce a pack */,
    std::index_sequence<II...>) {
    flog_assert(c.sizes.size() == sizeof...(VV),
      c.sizes.size() << " sizes for " << sizeof...(VV) << " index spaces");
    return {{data::partitioned(data::make_region<Policy, VV>(c.sizes[II]),
      c.parts,
      split(c.sizes[II], c.parts),
      data::disjoint,
      data::complete)...}};
  }
  static auto split(std::size_t n, std::size_t p) {
    return [=](std::size_t i) { return std::pair{i * n / p, (i + 1) * n / p}; };
  }
}; // struct canonical

template<class P>
template<std::size_t Priv>
struct canonical<P>::access {
  template<const auto & F>
  using accessor = data::accessor_member<F, Priv>;
  accessor<canonical::mine> mine;

  template<class F>
  void bind(F f) {
    f(mine);
  }
};

template<>
struct detail::base<canonical> {
  using type = canonical_base;
};

} // namespace topo
} // namespace flecsi
