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
#include "flecsi/data/field.hh"
#include "flecsi/execution.hh"
#include "flecsi/topo/index.hh"

// This will need to support different kind of constructors for
// src vs dst
// indirect vs direct
// indirect: range vs points

// indirect (point), direct
// indirect (point), indirect => mesh

namespace flecsi {
namespace data::detail {
struct intervals_base {
  using coloring = std::vector<std::size_t>;

protected:
  static std::size_t index(const coloring & c, std::size_t i) {
    return c[i];
  }
};
template<class P>
struct intervals_category : intervals_base, topo::repartitioned {
  explicit intervals_category(const coloring & c)
    : partitioned(
        topo::make_repartitioned<P>(c.size(), make_partial<index>(c))) {
    resize();
  }
};
} // namespace data::detail
// Needed before defining a specialization:
template<>
struct topo::detail::base<data::detail::intervals_category> {
  using type = data::detail::intervals_base;
};

namespace data {
namespace detail {
struct intervals : topo::specialization<intervals_category, intervals> {
  static const field<data::intervals::Value>::definition<intervals> field;
};
// Now that intervals is complete:
inline const field<data::intervals::Value>::definition<intervals>
  intervals::field;
} // namespace detail

struct copy_plan {
  using Sizes = detail::intervals::coloring;

  template<template<class> class C,
    class P,
    typename P::index_space S = P::default_space(),
    class D,
    class F>
  copy_plan(C<P> & t,
    const Sizes & ndests,
    D && dests,
    F && src,
    util::constant<S> = {})
    : reg(&t.template get_region<S>()), dest_ptrs_(ndests),
      // In this first case we use a subtopology to create the
      // destination partition which supposed to be contiguous
      dest_(*reg,
        dest_ptrs_,
        (std::forward<D>(dests)(detail::intervals::field(dest_ptrs_)),
          detail::intervals::field.fid)),
      ptr_fid(pointers<P, S>.fid),
      // From the pointers we feed in the destination partition
      // we create the source partition
      src_partition_(*reg,
        dest_,
        (std::forward<F>(src)(pointers<P, S>(t)), ptr_fid)) {}

  void issue_copy(const field_id_t & data_fid) const {
    launch_copy(*reg, src_partition_, dest_, data_fid, ptr_fid);
  }

private:
  const region * reg;
  detail::intervals::core dest_ptrs_;
  intervals dest_;
  field_id_t ptr_fid;
  points src_partition_;

  template<class T, typename T::index_space S>
  static inline const field<points::Value>::definition<T, S> pointers;
}; // struct copy_plan

} // namespace data
} // namespace flecsi
