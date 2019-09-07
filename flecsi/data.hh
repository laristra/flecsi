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

  This file contains the high-level, macro interface for the FleCSI
  data model.
 */

#include <flecsi/data/field.hh>
#include <flecsi/data/topology.hh>
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>

namespace flecsi {

/*
  Default global topology instance.
 */

inline const data::topology_need_name<topology::global_topology_t>
  global_topology_definition;
inline auto flecsi_global_topology = global_topology_definition();

/*
  Convenience type for global field members.
 */

template<typename DATA_TYPE>
using global_field_member = data::field_member<DATA_TYPE,
  data::storage_label_t::dense,
  topology::global_topology_t,
  0>;

/*
  Default index topology instance.
 */

inline const data::topology_need_name<topology::index_topology_t>
  index_topology_definition;
inline auto flecsi_index_topology = index_topology_definition();

/*
  Convenience type for index field members.
 */

template<typename DATA_TYPE>
using index_field_member = data::field_member<DATA_TYPE,
  data::storage_label_t::dense,
  topology::index_topology_t,
  0>;

} // namespace flecsi
