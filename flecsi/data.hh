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

#include <flecsi/data/accessor.hh>
#include <flecsi/data/coloring.hh>

namespace flecsi {

/*
  Default global topology instance.
 */

inline data::topology_slot<topology::global_t> global_topology;

/*
  Convenience type for global field members.
 */

template<typename DATA_TYPE>
using global_field_member = data::
  field_member<DATA_TYPE, data::storage_label_t::dense, topology::global_t, 0>;

/*
  Per-process coloring.
 */

inline data::coloring_slot<topology::index_t> process_coloring;

/*
  Per-process topology instance.
 */

inline data::topology_slot<topology::index_t> process_topology;

/*
  Convenience type for index field members.
 */

template<typename DATA_TYPE>
using index_field_member = data::
  field_member<DATA_TYPE, data::storage_label_t::dense, topology::index_t, 0>;

} // namespace flecsi
