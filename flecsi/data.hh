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

#include <flecsi/data/coloring.hh>
#include <flecsi/data/field.hh>
#include <flecsi/data/topology.hh>
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>

namespace flecsi {

/*
  Default global topology instance.
 */

inline const data::topology_reference<topology::global_topology_t>
  flecsi_global_topology;

/*
  Convenience type for global field members.
 */

template<typename DATA_TYPE>
using global_field_member = data::field_member<DATA_TYPE,
  data::storage_label_t::dense,
  topology::global_topology_t,
  0>;

/*
  Default index coloring.
 */

inline data::coloring_slot<topology::index_topology_t> flecsi_index_coloring;

/*
  Default index topology instance.
 */

inline const data::topology_reference<topology::index_topology_t>
  flecsi_index_topology;

/*
  Convenience type for index field members.
 */

template<typename DATA_TYPE>
using index_field_member = data::field_member<DATA_TYPE,
  data::storage_label_t::dense,
  topology::index_topology_t,
  0>;

} // namespace flecsi
