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
  region(size2, const fields &);

  size2 size() const;
};

struct partition {
  static auto make_row(std::size_t i, std::size_t n);
  using row = decltype(make_row(0, 0));

  explicit partition(const region &); // divides into rows
  // Derives row lengths from the field values (which should be of type row
  // and be equal in number to the rows).  The argument partition must survive
  // until this partition is updated or destroyed.
  partition(const region &,
    const partition &,
    field_id_t,
    completeness = incomplete);

  std::size_t colors() const;
  template<topo::single_space>
  const partition & get_partition() const {
    return *this;
  }
};
#endif

template<class Topo, typename Topo::index_space Index = Topo::default_space()>
region
make_region(size2 s) {
  return {s, run::context::instance().get_field_info_store<Topo, Index>()};
}

struct partitioned : region, partition {
  template<class... TT>
  partitioned(region && r, TT &&... tt)
    : region(std::move(r)),
      partition(static_cast<const region &>(*this), std::forward<TT>(tt)...) {}
};

template<class Topo, typename Topo::index_space Index = Topo::default_space()>
partitioned
make_partitioned(const partition & p,
  field_id_t fid,
  std::size_t n = logical_size,
  completeness cpt = incomplete) {
  return {make_region<Topo, Index>({p.colors(), n}), p, fid, cpt};
}

} // namespace flecsi::data
