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

#if !defined(FLECSI_ENABLE_CHARM)
#error FLECSI_ENABLE_CHARM not defined! This file depends on Charm!
#endif

#include "flecsi/run/backend.hh"
#include "flecsi/topo/core.hh" // single_space

namespace flecsi {
namespace data {
namespace charm {

// TODO: These are just placeholder definitions for region and partition
// while the topo interface for Flecsi is still in flux.
struct region {
  region(size2 s, const fields & fs) : s_(s) {}
  size2 size() const { return s_; }
  template<topo::single_space>
  region & get_region() {
    return *this;
  }
  template<class D>
  void cleanup(field_id_t f, D d) {}

  size2 s_;
};

struct partition {
  using row = std::size_t;
  static row make_row(std::size_t i, std::size_t n) {
    return i;
  }
  static std::size_t row_size(const row& r) {
    return 0;
  }
  partition(const region & reg) {}
  partition(const region & reg,
    const partition & src,
    field_id_t fid,
    completeness cpt = incomplete) {}

  std::size_t colors() const {
    // TODO: This may not be correct
    return CkNumPes();
  }

  template<topo::single_space>
  const partition & get_partition() const {
    return *this;
  }

  void
  update(const partition& src, field_id_t fid, completeness cpt = incomplete) {
  }
};
} // namespace charm

using charm::region, charm::partition; // for backend-agnostic interface

} // namespace data
} // namespace flecsi
