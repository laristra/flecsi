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
#include "flecsi/data/privilege.hh"
#include "flecsi/run/types.hh"

#include <set>

namespace flecsi::data {
#ifdef DOXYGEN // implemented per-backend
struct region_base {
  region(size2, const fields &);

  size2 size() const;
};

struct partition {
  static auto make_row(std::size_t i, std::size_t n);
  using row = decltype(make_row(0, 0));
  static std::size_t row_size(const row &);

  explicit partition(const region_base &); // divides into rows
  // Derives row lengths from the field values (which should be of type row
  // and be equal in number to the rows).  The argument partition must survive
  // until this partition is updated or destroyed.
  partition(const region_base &,
    const partition &,
    field_id_t,
    completeness = incomplete);

  std::size_t colors() const;
  void update(const partition &, field_id_t, completeness = incomplete);
  template<topo::single_space>
  const partition & get_partition(field_id_t) const {
    return *this;
  }
};
#endif

struct region : region_base {
  using region_base::region_base;

  std::set<field_id_t> dirty;
  // Return whether a copy is needed.
  template<std::size_t P>
  bool ghost(field_id_t i) {
    constexpr auto n = privilege_count(P);
    static_assert(n > 1, "need shared/ghost privileges");
    constexpr auto g = get_privilege(n - 1, P);
    constexpr bool gr = privilege_read(g),
                   sw = privilege_write(get_privilege(n - 2, P));
    // The logic here is constructed to allow a single read/write set access:
    // writing to ghosts, or reading from them without also writing to shared,
    // clears the dirty bit, and otherwise writing to shared sets it.
    // Otherwise, it retains its value (and we don't copy).
    return (privilege_write(g) || !sw && gr ? dirty.erase(i)
                                            : sw && !dirty.insert(i).second) &&
           gr;
  }

  template<topo::single_space>
  region & get_region() {
    return *this;
  }
};

template<class Topo, typename Topo::index_space Index = Topo::default_space()>
region
make_region(size2 s) {
  return {s, run::context::instance().get_field_info_store<Topo, Index>()};
}

template<class P>
struct partitioned : region, P {
  template<class... TT>
  partitioned(region && r, TT &&... tt)
    : region(std::move(r)),
      P(static_cast<const region &>(*this), std::forward<TT>(tt)...) {}
};

} // namespace flecsi::data
