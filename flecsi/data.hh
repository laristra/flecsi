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

inline data::topology_slot<topology::global> global_topology;

/*
  Convenience type for global field members.
 */

template<typename DATA_TYPE>
using global_field_member =
  data::field_member<DATA_TYPE, data::dense, topology::global, 0>;

/*
  Per-process coloring.
 */

inline data::coloring_slot<topology::index> process_coloring;

/*
  Per-process topology instance.
 */

inline data::topology_slot<topology::index> process_topology;

/*
  Convenience type for index field members.
 */

template<typename DATA_TYPE>
using index_field_member =
  data::field_member<DATA_TYPE, data::dense, topology::index, 0>;

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
      process_coloring.allocate(runtime::context_t::instance().processes());
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
