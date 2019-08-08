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

#include <flecsi/data/common/privilege.hh>
#include <flecsi/data/field.hh>
#include <flecsi/data/topology_interface.hh>
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>
#include <flecsi/utils/const_string.hh>

namespace flecsi {

#if 0 // working to fix this
  flecsi_topology_reference(                                                   \
    flecsi::topology::index_topology_t, "internal", "index_topology")
#endif

template<typename DATA_TYPE>
using global_field_member_u = data::field_member_u<DATA_TYPE,
  data::storage_label_t::dense,
  topology::global_topology_t,
  0>;

template<typename DATA_TYPE>
using index_field_member_u = data::field_member_u<DATA_TYPE,
  data::storage_label_t::dense,
  topology::index_topology_t,
  0>;

} // namespace flecsi

/*----------------------------------------------------------------------------*
  General Topology Interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_topology_reference

  Declare a variable of type \em type with namespace \em nspace
  and name \em name.

  This macro returns a reference to a topology instance. This call does not
  necessarily cause memory to be allocated. It's primary function is to
  describe the topology type to the runtime. Memory allocation will likely be
  deferred.

  @param type   The topology type.
  @param nspace The string namespace to use to register the variable.
  @param name   The string name to use to register the variable.

  @ingroup data
 */

#define flecsi_topology_reference(type, nspace, name)                          \
  flecsi::data::topology_interface_t::reference<type,                          \
    flecsi_internal_string_hash(nspace),                                       \
    flecsi_internal_string_hash(name)>({flecsi_internal_stringify(name)})

#define flecsi_index_topology                                                  \
  flecsi_topology_reference(                                                   \
    flecsi::topology::index_topology_t, "internal", "index_topology")

#define flecsi_global_topology                                                 \
  flecsi_topology_reference(                                                   \
    flecsi::topology::global_topology_t, "internal", "global_topology")
