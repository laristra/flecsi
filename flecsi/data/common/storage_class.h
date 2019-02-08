/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!
  @file

  This file defines the base \em storage_class_u type that can be
  specialized by specific storage class, and by data client type.

  This file also defines the storage classes for the internal \em global
  and \em color client types.
 */

#include <flecsi/data/common/client_handle.h>

#ifndef POLICY_NAMESPACE
#error You must define a data policy namespace before including this file.
#endif

namespace flecsi {
namespace data {

/*!
  The storage_label_t type enumerates the available FleCSI storage classes.
  A FleCSI storage class provides a specific interface for different
  logical data layouts, e.g., dense vs. sparse. The actual data layout
  is implementation dependent.
 */

enum storage_label_t : size_t {
  global,
  color,
  dense,
  sparse,
  ragged,
  subspace
}; // enum storage_label_t

namespace POLICY_NAMESPACE {

/*!
  Base storage class type for client-specific specializations.
 */

template<size_t STORAGE_CLASS, typename CLIENT_TYPE>
struct storage_class_u {};

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

namespace global_topology {

template<typename TYPE, size_t PRIVILEGES>
struct handle_u {

  handle_u() {}
  ~handle_u() {}

}; // struct handle_u

} // namespace global_topology

template<>
struct storage_class_u<global, flecsi::topology::global_topology_t> {

  using client_t = flecsi::topology::global_topology_t;
  using client_handle_t = client_handle_u<client_t, 0>;
  using handle_t = global_topology::handle_u<client_t, 0>;

  template<typename DATA_TYPE, size_t NAMESPACE, size_t NAME, size_t VERSION>
  static handle_t get_handle(const client_handle_t & client_handle) {
    handle_t h;
    return h;
  } // get_handle

}; // struct storage_class_u

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

namespace color_topology {

template<typename TYPE, size_t PRIVILEGES>
struct handle_u {

  handle_u() {}
  ~handle_u() {}

}; // struct handle_u

} // namespace color_topology

template<>
struct storage_class_u<color, flecsi::topology::color_topology_t> {

  using client_t = flecsi::topology::global_topology_t;
  using client_handle_t = client_handle_u<client_t, 0>;
  using handle_t = color_topology::handle_u<client_t, 0>;

  template<typename DATA_TYPE, size_t NAMESPACE, size_t NAME, size_t VERSION>
  static handle_t get_handle(const client_handle_t & client_handle) {
    handle_t h;
    return h;
  } // get_handle

}; // struct storage_class_u

} // namespace POLICY_NAMESPACE
} // namespace data
} // namespace flecsi
