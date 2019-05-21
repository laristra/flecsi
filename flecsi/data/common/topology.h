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

 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/data/common/data_reference.h>
#endif

#ifndef POLICY_NAMESPACE
#error You must define a data policy namespace before including this file.
#endif

namespace flecsi {
namespace data {
namespace POLICY_NAMESPACE {

/*!
  Base topology type for topology-specific specializations. Specializations of
  this type must implement a get_reference method.
 */

template<typename TOPOLOGY_TYPE>
struct topology_u {

  using topology_reference_t = topology_reference_u<TOPOLOGY_TYPE>;

  template<size_t NAMESPACE, size_t NAME>
  static topology_reference_t get_reference() {
    constexpr size_t identifier = utils::hash::topology_hash<NAMESPACE, NAME>();
    return {identifier};
  } // get_reference
};

template<typename TOPOLOGY_TYPE>
struct topology_instance_u {};

} // namespace POLICY_NAMESPACE
} // namespace data
} // namespace flecsi
