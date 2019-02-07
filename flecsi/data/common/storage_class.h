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
#include <flecsi/data/common/storage_label.h>

#ifndef POLICY_NAMESPACE
#error You must define a data policy namespace before including this file.
#endif

namespace flecsi {
namespace data {
namespace POLICY_NAMESPACE {

template<size_t STORAGE_CLASS, typename CLIENT_TYPE>
struct storage_class_u {};

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

namespace global_topology {

template<typename TYPE, size_t PRIVILEGES>
struct dense_handle_u {

  dense_handle_u() {}
  ~dense_handle_u() {}

}; // struct dense_handle_u

} // namespace global_topology

template<>
struct storage_class_u<dense, flecsi::topology::global_topology_t> {

  using client_t = flecsi::topology::global_topology_t;
  using client_handle_t = client_handle_u<client_t, 0>;
  using handle_t = global_topology::dense_handle_u<client_t, 0>;

  template<typename DATA_TYPE, size_t NAMESPACE, size_t NAME, size_t VERSION>
  static handle_t get_handle(const client_handle_t & client_handle) {
    handle_t h;
    return h;
  } // get_handle

}; // struct storage_class_u

} // namespace POLICY_NAMESPACE
} // namespace data
} // namespace flecsi
