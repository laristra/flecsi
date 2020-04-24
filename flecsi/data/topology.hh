/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2020, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/data/backend.hh"

namespace flecsi::data {
#ifdef DOXYGEN // implemented per-backend
struct region {
  region(std::size_t, const fields &);
};

struct partition {
  template<class F> // F: int -> [start, end)
  partition(const region &,
    std::size_t,
    F,
    disjointness = {},
    completeness = {});
  std::size_t colors() const;
  template<topo::single_space>
  const partition & get_partition() const {
    return *this;
  }
};
#endif

template<class Topo,
  topo::index_space_t<Topo> Index = topo::default_space<Topo>>
region
make_region(std::size_t n) {
  return {n, run::context::instance().get_field_info_store<Topo, Index>()};
}

template<class Topo>
struct simple : region {
  using type = Topo;
  simple(std::size_t n = 1) : region(make_region<type>(n)) {}
};

} // namespace flecsi::data
