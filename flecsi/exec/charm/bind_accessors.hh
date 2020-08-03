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
#include "flecsi/exec/charm/future.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/util/demangle.hh"
#include "flecsi/util/tuple_walker.hh"

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {

inline log::devel_tag bind_accessors_tag("bind_accessors");

namespace exec::charm {

/*!
  The bind_accessors_t type is called to walk the user task arguments inside of
  an executing legion task to properly complete the users accessors, i.e., by
  pointing the accessor \em view instances to the appropriate legion-mapped
  buffers.
 */

struct bind_accessors_t : public util::tuple_walker<bind_accessors_t> {

  /*!
    Construct an bind_accessors_t instance.

    @param legion_runtime The Legion task runtime.
    @param legion_context The Legion task runtime context.
   */

  bind_accessors_t(std::vector<std::byte>& buf)
    : buf_(buf) {}

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<data::dense, DATA_TYPE, PRIVILEGES> & accessor) {
    //auto & reg = regions_[region++];

    //    Legion::FieldAccessor<privilege_mode(get_privilege<0, PRIVILEGES>()),
    /*const Legion::UnsafeFieldAccessor<DATA_TYPE,
      1,
      Legion::coord_t,
      Realm::AffineAccessor<DATA_TYPE, 1, Legion::coord_t>>
      ac(reg, accessor.identifier(), sizeof(DATA_TYPE));
    const auto dom = legion_runtime_->get_index_space_domain(
      legion_context_, reg.get_logical_region().get_index_space());
    const auto r = dom.get_rect<1>();*/
    flog_assert(buf_.size() % sizeof(DATA_TYPE) == 0, "Bad buffer size\n");
    auto & flecsi_context = run::context::instance();
    DATA_TYPE* d = (DATA_TYPE*)flecsi_context.getField(accessor.identifier());
    bind(accessor, 1, d);

    /*bind(accessor,
      r.hi[0] - r.lo[0] + 1,
      ac.ptr(Legion::Domain::DomainPointIterator(dom).p));*/
  }

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<data::singular, DATA_TYPE, PRIVILEGES> & accessor) {
    visit(accessor.get_base());
  }

  template<class Topo, std::size_t Priv>
  void visit(data::topology_accessor<Topo, Priv> & a) {
    a.bind([&](auto & x) { visit(x); }); // Clang 8.0.1 deems 'this' unused
  }

  /*--------------------------------------------------------------------------*
   Futures
   *--------------------------------------------------------------------------*/
  template<typename DATA_TYPE>
  void visit(exec::flecsi_future<DATA_TYPE, launch_type_t::single> & future) {
    /*future.legion_future_ = futures_[future_id];
    future_id++;*/
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
  std::vector<std::byte>& buf_;

}; // struct bind_accessors_t

} // namespace exec::charm
} // namespace flecsi
