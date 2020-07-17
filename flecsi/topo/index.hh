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

#include "flecsi/data/field.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/topo/core.hh"

namespace flecsi {
namespace topo {

struct index_base {
  struct coloring {
    coloring(size_t size) : size_(size) {}

    size_t size() const {
      return size_;
    }

  private:
    size_t size_;
  };
};

template<class P>
struct index_category : index_base, data::partitioned {
  index_category(const coloring & c)
    : partitioned(data::make_region<P>({c.size(), 1})) {}
};
template<>
struct detail::base<index_category> {
  using type = index_base;
};

struct resize {
  explicit resize(std::size_t n) : size(n) {}

  auto operator()() const {
    return field(size.get_slot());
  }
  template<class Topo, typename Topo::index_space S = Topo::default_space()>
  auto make_partitioned() const {
    return data::make_partitioned<Topo, S>(*size, field.fid);
  }

  using Field = flecsi::field<data::partition::row, data::singular>;

private:
  // cslot can't be used, but is unneeded.
  struct topo : specialization<index_category, topo> {};
  static inline const Field::definition<topo> field;
  data::anti_slot<topo> size;
};

// To control initialization order:
struct with_size {
  with_size(std::size_t n) : sizes(n) {}
  resize sizes;
};

/*!
  The \c index type allows users to register data on an
  arbitrarily-sized set of indices that have an implicit one-to-one coloring.

  @ingroup topology
 */
struct index : specialization<index_category, index> {
  static coloring color(size_t size) {
    return {size};
  } // color

}; // struct index

} // namespace topo
} // namespace flecsi
