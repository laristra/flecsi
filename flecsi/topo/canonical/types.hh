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

#include "flecsi/topo/index.hh"

#include <cstddef>
#include <vector>

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

protected:
  using meta_topo = meta_topology<canonical_base>;

public:
  static inline const field<Meta, data::singular>::definition<meta_topo>
    meta_field;

  // For this simple case, two scalars determine all colors' sizes.
  static std::size_t allocate(std::size_t n, std::size_t p, std::size_t i) {
    return (i + 1) * n / p - i * n / p;
  }
}; // struct canonical_base

} // namespace topo
} // namespace flecsi
