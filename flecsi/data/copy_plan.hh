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
// NB: Registering a field on an indirect topology doesn't do anything.
struct indirect_base : partition {
  using coloring = void;

  template<class... AA>
  explicit indirect_base(region * r, AA &&... aa)
    : partition(*r, std::forward<AA>(aa)...), reg(r) {}

  template<topo::single_space = topo::elements>
  region & get_region() const {
    return *reg;
  }

private:
  region * reg;
};
template<class>
struct indirect_category : indirect_base {
  using indirect_base::indirect_base;
};
} // namespace data::detail
// Needed before defining a specialization:
template<>
struct topo::detail::base<data::detail::indirect_category> {
  using type = data::detail::indirect_base;
};

namespace data {
namespace detail {
struct indirect : topo::specialization<indirect_category, indirect> {};
} // namespace detail

struct copy_plan : private topo::with_size {
  using Points = std::vector<std::vector<data::partition::point>>;

  static void fill_dst_sizes_task(topo::resize::Field::accessor<wo> a,
    const std::vector<data::partition::row> & v) {
    const auto i = run::context::instance().color();
    a = v[i];
  }

  static void fill_dst_ptrs_task(
    flecsi::field<data::partition::point>::accessor<wo> a,
    const std::vector<std::vector<data::partition::point>> & v) {
    const auto c = run::context::instance().color();
    assert(v[c].size() == a.span().size());
    std::copy(v[c].begin(), v[c].end(), a.span().begin());
  }

  template<template<class> class C,
    class P,
    typename P::index_space S = P::default_space()>
  copy_plan(C<P> & t,
    const std::vector<data::partition::row> & dst_row_vec,
    const Points & src,
    util::constant<S> = {})
    : with_size(t.colors()),
      // In this first case we use a subtopology to create the
      // destination partition which supposed to be contiguous
      dest_(&t.template get_region<S>(),
        sz,
        (execute<fill_dst_sizes_task>(sizes(), dst_row_vec),
          topo::resize::field.fid)),
      // From the pointers we feed in the destination partition
      // we create the source partition
      src_partition_(dest_.get_region(),
        dest_,
        (execute<fill_dst_ptrs_task>(indirect_register<P, S>(dest_), src),
          pointers.fid),
        data::partition::buildByImage_tag) {}

  void issue_copy(const field_id_t & data_fid) const {
    launch_copy(
      dest_.get_region(), src_partition_, dest_, data_fid, pointers.fid);
  }

private:
  detail::indirect::core dest_;
  partition src_partition_;

  static inline const field<partition::point>::definition<detail::indirect>
    pointers;
  template<class T, typename T::index_space S>
  static inline auto & indirect_register =
    (run::context::instance().add_field_info<T, S>(&pointers), pointers);
}; // struct copy_plan

} // namespace data
} // namespace flecsi
