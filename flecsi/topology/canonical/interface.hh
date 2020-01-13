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

#include <flecsi/topology/canonical/types.hh>

#include <string>

namespace flecsi {
namespace topology {

/*!
  The canonical type is a dummy topology for development and testing.

  @ingroup topology
 */

template<typename Policy>
struct canonical : canonical_base {

  using coloring = canonical_base::coloring;

  canonical() = delete;

  template<typename... ARGS>
  static coloring color(ARGS &&... args) {
    return Policy::color(std::forward<ARGS>(args)...);
  } // color

}; // struct canonical

} // namespace topology
} // namespace flecsi
