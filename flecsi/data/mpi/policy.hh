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

#include <cstddef>

#include "flecsi/data/field_info.hh"
#include "flecsi/topo/core.hh" // single_space

namespace flecsi {
namespace data {

namespace detail {
struct region {
  region(std::size_t, const fields &) {}
};

struct partition {
  template<class F>
  partition(const region &,
    std::size_t,
    F,
    disjointness = {},
    completeness = {}) {}
  std::size_t colors() const {
    return 0;
  }
  template<topo::single_space>
  const partition & get_partition() const {
    return *this;
  }
};
} // namespace detail

} // namespace data
} // namespace flecsi
