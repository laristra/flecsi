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

namespace flecsi::data {

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
    : with_size(t.colors()), reg_(t.template get_region<S>()),
      ptr_field_id_(pointers<P, S>.fid),
      // In this first case we use a subtopology to create the
      // destination partition which supposed to be contiguous
      dst_partition_(reg_,
        sz,
        (execute<fill_dst_sizes_task>(sizes(), dst_row_vec),
          topo::resize::field.fid)),
      // From the pointers we feed in the destination partition
      // we create the source partition
      src_partition_(reg_,
        dst_partition_,
        (execute<fill_dst_ptrs_task>(pointers<P, S>(t), src), ptr_field_id_),
        data::partition::buildByImage_tag) {}

  void issue_copy(const field_id_t & data_fid) {
    launch_copy(reg_, src_partition_, dst_partition_, data_fid, ptr_field_id_);
  }

  field_id_t field_id() const {
    return ptr_field_id_;
  }
  const partition & get_dst_partition() const {
    return dst_partition_;
  }

private:
  const region & reg_;
  field_id_t ptr_field_id_;
  partition dst_partition_;
  partition src_partition_;

  template<class T, typename T::index_space S>
  static inline const field<partition::point>::definition<T, S> pointers;

  // Subtopology

}; // struct copy_plan

} // namespace flecsi::data