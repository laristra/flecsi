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
  static constexpr std::size_t index_spaces = Policy::index_spaces;

  canonical(const coloring &) {}
}; // struct canonical

template<>
struct detail::base<canonical> {
  using type = canonical_base;
};

} // namespace topo
} // namespace flecsi
