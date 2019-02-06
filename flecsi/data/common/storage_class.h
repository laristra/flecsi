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

/*! @file */

#include <flecsi/data/common/client_handle.h>

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

struct storage_class_u<dense, flecsi::topology::global_topology_t> {

  using client_t = flecsi::topology::global_topology_t;

  template<typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static global_topology::handle_t<DATA_TYPE, 0> get_handle(
    const client_handle_u<client_t, 0> & client_handle) {
  } // get_handle

}; // struct storage_class_u

} // namespace POLICY_NAMESPACE
} // namespace data
} // namespace flecsi
