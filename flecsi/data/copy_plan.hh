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
struct topo::detail::base<data::detail::intervals_category> {
  using type = data::detail::intervals_base;
};
template<>
struct topo::detail::base<data::detail::indirect_category> {
  using type = data::detail::indirect_base;
};

namespace data {
namespace detail {
struct intervals : topo::specialization<intervals_category, intervals> {
  static const field<partition::row>::definition<intervals> field;
};
// Now that intervals is complete:
inline const field<partition::row>::definition<intervals> intervals::field;
struct indirect : topo::specialization<indirect_category, indirect> {};
} // namespace detail

struct copy_plan {
  using Intervals = std::vector<std::vector<subrow>>;
  using Points = std::vector<std::vector<data::partition::point>>;

  static void set_dests(field<partition::row>::accessor<wo> a,
    const Intervals & v) {
    const auto i = color();
    auto * p = a.span().data();
    for(auto & s : v[i])
      *p++ = partition::make_row(i, s);
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
    const Intervals & dests,
    const Points & src,
    util::constant<S> = {})
    : dest_ptrs_([&dests] {
        detail::intervals::coloring ret;
        ret.reserve(dests.size());
        for(const auto & v : dests)
          ret.push_back(v.size());
        return ret;
      }()),
      // In this first case we use a subtopology to create the
      // destination partition which supposed to be contiguous
      dest_(&t.template get_region<S>(),
        dest_ptrs_,
        (execute<set_dests>(detail::intervals::field(dest_ptrs_), dests),
          detail::intervals::field.fid)),
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
  detail::intervals::core dest_ptrs_;
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
