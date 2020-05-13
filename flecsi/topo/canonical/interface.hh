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
  template<std::size_t>
  struct access;

  template<class F>
  static void fields(F f) {
    f(mine);
  }

  canonical(const coloring & c)
    : vert(data::make_region<Policy, vertices>(c.size + 1),
        c.parts,
        split(c.size + 1, c.parts),
        data::disjoint,
        data::complete),
      cell(data::make_region<Policy, cells>(c.size),
        c.parts,
        split(c.size, c.parts),
        data::disjoint,
        data::complete) {}

  static inline const field<int>::definition<Policy, cells> mine;

  data::partitioned vert, cell;

  template<index_space S>
  auto & get_partition() const {
    return S == cells ? cell : vert;
  }

private:
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
