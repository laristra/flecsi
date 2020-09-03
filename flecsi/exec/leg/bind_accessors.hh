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

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/data/accessor.hh"
#include "flecsi/data/privilege.hh"
#include "flecsi/data/topology_accessor.hh"
#include "flecsi/exec/leg/future.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/util/demangle.hh"
#include "flecsi/util/tuple_walker.hh"

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <memory>

namespace flecsi {

inline log::devel_tag bind_accessors_tag("bind_accessors");

namespace exec::leg {

/*!
  The bind_accessors type is called to walk the user task arguments inside of
  an executing legion task to properly complete the users accessors, i.e., by
  pointing the accessor \em view instances to the appropriate legion-mapped
  buffers.
 */

struct bind_accessors : public util::tuple_walker<bind_accessors> {

  /*!
    Construct an bind_accessors instance.

    @param legion_runtime The Legion task runtime.
    @param legion_context The Legion task runtime context.
   */

  bind_accessors(Legion::Runtime * legion_runtime,
    Legion::Context & legion_context,
    std::vector<Legion::PhysicalRegion> const & regions,
    std::vector<Legion::Future> const & futures)
    : legion_runtime_(legion_runtime), legion_context_(legion_context),
      regions_(regions), futures_(futures) {}

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<data::raw, DATA_TYPE, PRIVILEGES> & accessor) {
    auto & reg = regions_[region++];

    //    Legion::FieldAccessor<privilege_mode(get_privilege<0, PRIVILEGES>()),
    const Legion::UnsafeFieldAccessor<DATA_TYPE,
      2,
      Legion::coord_t,
      Realm::AffineAccessor<DATA_TYPE, 2, Legion::coord_t>>
      ac(reg, accessor.identifier(), sizeof(DATA_TYPE));
    const auto dom = legion_runtime_->get_index_space_domain(
      legion_context_, reg.get_logical_region().get_index_space());
    const auto r = dom.get_rect<2>();

    accessor.bind(util::span(ac.ptr(Legion::Domain::DomainPointIterator(dom).p),
      r.hi[1] - r.lo[1] + 1));
  }

  template<typename T, size_t P>
  void visit(data::accessor<data::dense, T, P> & a) {
    visit(a.get_base());
    if constexpr(privilege_write_only(P))
      construct(a);
  }
  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<data::singular, DATA_TYPE, PRIVILEGES> & accessor) {
    visit(accessor.get_base());
  }
  // Without a catch-all, this matches accessor<ragged|sparse,...>.
  template<class T, std::size_t P, std::size_t OP>
  void visit(data::ragged_accessor<T, P, OP> & a) {
    visit(a.get_base());
    visit(a.get_offsets());
    if constexpr(privilege_write_only(P))
      construct(a);
  }
  template<class T>
  void visit(data::mutator<data::ragged, T> & m) {
    visit(m.get_base());
    m.bind();
  }
  template<class T>
  void visit(data::mutator<data::sparse, T> & m) {
    visit(m.get_base());
  }

  template<class Topo, std::size_t Priv>
  void visit(data::topology_accessor<Topo, Priv> & a) {
    a.bind([&](auto & x) { visit(x); }); // Clang 8.0.1 deems 'this' unused
  }

  /*--------------------------------------------------------------------------*
   Futures
   *--------------------------------------------------------------------------*/
  template<typename DATA_TYPE>
  void visit(future<DATA_TYPE> & f) {
    f = {futures_[future_id++]};
  }

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE>
  static typename std::enable_if_t<
    !std::is_base_of_v<data::reference_base, DATA_TYPE>>
  visit(DATA_TYPE &) {
    {
      log::devel_guard guard(bind_accessors_tag);
      flog_devel(info) << "Skipping argument with type "
                       << util::type<DATA_TYPE>() << std::endl;
    }
  } // visit

private:
  template<class A>
  static void construct(const A & a) {
    const auto s = a.span();
    std::uninitialized_default_construct(s.begin(), s.end());
  }

  Legion::Runtime * legion_runtime_;
  Legion::Context & legion_context_;
  size_t region = 0;
  const std::vector<Legion::PhysicalRegion> & regions_;
  size_t future_id = 0;
  const std::vector<Legion::Future> & futures_;

}; // struct bind_accessors

} // namespace exec::leg
} // namespace flecsi
