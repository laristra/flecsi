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

#include "flecsi/topo/core.hh"

namespace flecsi {
namespace topo {

struct global_base {
  struct coloring {};
  global_base() = delete;
};

template<class>
using global_category = global_base;

/*!
  The \c global type allows users to register data on a
  topology with a single index, i.e., there is one instance of
  the registered field type that is visible to all colors.

  @ingroup topology
 */
struct global : specialization<global_category, global> {}; // struct global

} // namespace topo
} // namespace flecsi
