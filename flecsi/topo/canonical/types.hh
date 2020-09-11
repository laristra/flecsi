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

  struct Meta {
    util::id column_size, column_offset;
  };

  static std::size_t allocate(std::size_t n, std::size_t p, std::size_t i) {
    return (i + 1) * n / p - i * n / p;
  }
}; // struct canonical_base

} // namespace topo
} // namespace flecsi
