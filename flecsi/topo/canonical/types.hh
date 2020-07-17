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

#include <cstddef>
#include <vector>

#include "flecsi/topo/index.hh"

namespace flecsi {
namespace topo {

struct canonical_base {

  struct coloring {
    std::vector<std::size_t> sizes;
    std::size_t parts;
  }; // struct coloring

  static void
  allocate(resize::Field::accessor<wo> a, std::size_t n, std::size_t p) {
    const auto i = color();
    a = data::partition::make_row(i, (i + 1) * n / p - i * n / p);
  }
}; // struct canonical_base

} // namespace topo
} // namespace flecsi
