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

#include "flecsi/data/reference.hh"
#include <flecsi/topology/structured/types.hh>

namespace flecsi {
namespace topology {

/*!
  @ingroup topology
 */

template<typename Policy>
struct structured : structured_base {

  using coloring = structured_base::coloring;

  structured() = delete;

  template<typename... ARGS>
  static coloring color(ARGS &&... args) {
    return Policy::color(std::forward<ARGS>(args)...);
  } // color

}; // struct structured

} // namespace topology
} // namespace flecsi
