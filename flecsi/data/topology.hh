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

#include <cstddef>
#include <utility>

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

template<class Topo, typename Topo::index_space Index = Topo::default_space()>
region
make_region(std::size_t n) {
  return {n, run::context::instance().get_field_info_store<Topo, Index>()};
}

struct partitioned : region, partition {
  template<class... TT>
  partitioned(region && r, TT &&... tt)
    : region(std::move(r)), partition(*this, std::forward<TT>(tt)...) {}
};

} // namespace flecsi::data
