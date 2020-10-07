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

namespace mpi {
struct region {
  region(size2, const fields &) {}

  size2 size() const {
    return {};
  }
};

struct partition {
  using row = std::size_t;
  static row make_row(std::size_t, std::size_t n) {
    return n;
  }
  static std::size_t row_size(const row & r) {
    return r;
  }

  explicit partition(const region &) {}
  partition(const region &,
    const partition &,
    field_id_t,
    completeness = incomplete) {}

  std::size_t colors() const {
    return 0;
  }
  void update(const partition &, field_id_t, completeness = incomplete) {}
  template<topo::single_space>
  const partition & get_partition(field_id_t) const {
    return *this;
  }
};
} // namespace mpi

// For backend-agnostic interface:
using region_base = mpi::region;
using mpi::partition;

} // namespace data
} // namespace flecsi
