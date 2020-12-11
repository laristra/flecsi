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
#include "flecsi/exec/leg/future.hh"

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {

namespace topo {
struct global_base;
} // namespace topo

namespace exec {

struct task_prologue {
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
    return r ? w ? READ_WRITE : READ_ONLY
             : w ? privilege_discard(mode) ? WRITE_DISCARD : WRITE_ONLY
                 : NO_ACCESS;
  } // privilege_mode

protected:
  // This implementation can be generic because all topologies are expected to
  // provide get_region (and, with one exception, get_partition).
  template<typename DATA_TYPE, size_t PRIVILEGES, auto Space, class Topo>
  void visit(data::accessor<data::raw, DATA_TYPE, PRIVILEGES> &,
    const data::field_reference<DATA_TYPE, data::raw, Topo, Space> & r) {
    const field_id_t f = r.fid();
    auto & t = r.topology();
    data::region & reg = t.template get_region<Space>();

    reg.ghost_copy<PRIVILEGES>(r);

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

  /*--------------------------------------------------------------------------*
    Futures
   *--------------------------------------------------------------------------*/
  template<class P, class T>
  void visit(P &, const future<T> & f) {
    futures_.push_back(f.legion_future_);
  }

  template<class P, class T>
  void visit(P &, const future<T, exec::launch_type_t::index> & f) {
    future_maps_.push_back(f.legion_future_);
  }

private:
  std::vector<Legion::RegionRequirement> region_reqs_;
  std::vector<Legion::Future> futures_;
  std::vector<Legion::FutureMap> future_maps_;
};

} // namespace exec
} // namespace flecsi
