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
#include <flecsi/data/common/data_reference.hh>
#endif

namespace flecsi {
namespace data {

#if 0 // FIXME: Remove this
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
#endif

template<typename TOPOLOGY_TYPE>
struct topology_instance_u {};

} // namespace data
} // namespace flecsi
