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

#include "flecsi/data/field.hh"
#include "flecsi/data/privilege.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/data/topology_accessor.hh"
#include "flecsi/exec/leg/future.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/topo/set/interface.hh"
#include "flecsi/topo/structured/interface.hh"
//#include "flecsi/topo/unstructured/interface.hh"
#include "flecsi/util/demangle.hh"
#include "flecsi/util/tuple_walker.hh"

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {

namespace topo {
struct global_base;
} // namespace topo

inline log::devel_tag task_prologue_tag("task_prologue");

namespace exec::leg {

/*!
  The task_prologue type can be called to walk task args before the
  task launcher is created. This allows us to gather region requirements
  and to set state on the associated data handles \em before Legion gets
  the task arguments tuple.

  @ingroup execution
*/

struct task_prologue {
  // Accessors here are the empty versions made to be serialized.
  template<class P, class... AA>
  task_prologue(P & p, AA &... aa) {
    std::apply([&](auto &... pp) { (visit(&pp, aa), ...); }, p);
  }

  std::vector<Legion::RegionRequirement> const & region_requirements() const {
    return region_reqs_;
  } // region_requirements

  std::vector<Legion::Future> && futures() && {
    return std::move(futures_);
  } // futures

  std::vector<Legion::FutureMap> const & future_maps() const {
    return future_maps_;
  } // future_maps

private:
  /*!
    Convert the template privileges to proper Legion privileges.

    @param mode privilege
   */

  static Legion::PrivilegeMode privilege_mode(size_t mode) {
    // Reduce the read and write permissions for each privilege separately:
    bool r = false, w = false;
    for(auto i = privilege_count(mode); i-- && !(r && w);) {
      const auto p = get_privilege(i, mode);
      r = r || privilege_read(p);
      w = w || privilege_write(p);
    }
    return r ? w ? READ_WRITE : READ_ONLY : w ? WRITE_DISCARD : NO_ACCESS;
  } // privilege_mode

  template<class A>
  auto visitor(A & a) {
    return
      [&](auto & p, auto && f) { visit(&p, std::forward<decltype(f)>(f)(a)); };
  }

  // All fields are handled in terms of their underlying raw-layout fields.
  // Other (topology) accessors provide a send function that decomposes them
  // (and any associated argument).

  // This implementation can be generic because all topologies are expected to
  // provide get_region (and, with one exception, get_partition).
  template<typename DATA_TYPE, size_t PRIVILEGES, auto Space, class Topo>
  void visit(data::accessor<data::raw, DATA_TYPE, PRIVILEGES> * /* parameter */,
    const data::field_reference<DATA_TYPE, data::raw, Topo, Space> & r) {
    const field_id_t f = r.fid();
    auto & t = r.topology();
    data::region & reg = t.template get_region<Space>();

    constexpr auto np = privilege_count(PRIVILEGES);
    static_assert(np == Topo::template privilege_count<Space>,
      "privilege-count mismatch between accessor and topology type");
    if constexpr(np > 1) {
      if(reg.ghost<PRIVILEGES>(f))
        t.template ghost_copy<Space>(f);
    }

    const Legion::PrivilegeMode m =
      reg.poll_discard(f) ? WRITE_DISCARD : privilege_mode(PRIVILEGES);
    const Legion::LogicalRegion lr = reg.logical_region;
    if constexpr(std::is_same_v<typename Topo::base, topo::global_base>)
      region_reqs_.emplace_back(lr, m, EXCLUSIVE, lr);
    else
      region_reqs_.emplace_back(
        t.template get_partition<Space>(f).logical_partition,
        0,
        m,
        EXCLUSIVE,
        lr);
    region_reqs_.back().add_field(f);
  } // visit

  template<class P, class A>
  std::enable_if_t<std::is_base_of_v<data::send_tag, P>> visit(P * p, A && a) {
    p->send(visitor(a));
  }

  /*--------------------------------------------------------------------------*
    Futures
   *--------------------------------------------------------------------------*/
  template<class P, class T>
  void visit(P *, const future<T> & f) {
    futures_.push_back(f.legion_future_);
  }

  template<class P, class T>
  void visit(P *, const future<T, exec::launch_type_t::index> & f) {
    future_maps_.push_back(f.legion_future_);
  }

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<class P, class A>
  static std::enable_if_t<!std::is_base_of_v<data::convert_tag, A>> visit(P *,
    const A &) {
    log::devel_guard guard(task_prologue_tag);
    flog_devel(info) << "Skipping argument with type " << util::type<A>()
                     << std::endl;
  } // visit

  std::vector<Legion::RegionRequirement> region_reqs_;
  std::vector<Legion::Future> futures_;
  std::vector<Legion::FutureMap> future_maps_;
};

} // namespace exec::leg
} // namespace flecsi
