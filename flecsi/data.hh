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

/*!
  @file

  User interface to the FleCSI
  data model.
 */

#include "flecsi/data/topology.hh"
#include "flecsi/data/topology_slot.hh"
#include <flecsi/data/accessor.hh>
#include <flecsi/data/coloring.hh>

namespace flecsi {

/*
  Default global topology instance.
 */
inline topo::global::slot global_topology;

/*
  Per-process coloring.
 */
inline topo::index::cslot process_coloring;

/*
  Per-process topology instance.
 */
inline topo::index::slot process_topology;

namespace detail {
/// An RAII type to manage the global coloring and topologies.
struct data_guard {
  struct global_guard {
    global_guard() {
      global_topology.allocate({});
    }
    global_guard(global_guard &&) = delete;
    ~global_guard() {
      global_topology.deallocate();
    }
  } g;
  struct color_guard {
    color_guard() {
      process_coloring.allocate(run::context::instance().processes());
    }
    color_guard(color_guard &&) = delete;
    ~color_guard() {
      process_coloring.deallocate();
    }
  } c;
  struct process_guard {
    process_guard() {
      process_topology.allocate(process_coloring.get());
    }
    process_guard(process_guard &&) = delete;
    ~process_guard() {
      process_topology.deallocate();
    }
  } p;
};
} // namespace detail

} // namespace flecsi
