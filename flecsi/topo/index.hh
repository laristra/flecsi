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
struct index_category : index_base, data::simple<P>, data::partition {
  index_category(const coloring & c)
    : index_category::simple(c.size()), partition(
                                          *this,
                                          c.size(),
                                          [](std::size_t i) {
                                            return std::pair{i, i + 1};
                                          },
                                          data::disjoint,
                                          data::complete) {}
};
template<>
struct detail::base<index_category> {
  using type = index_base;
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
