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

/*!  @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include "data_reference.hh"
#endif

namespace flecsi {
namespace data {

/*!
  Topology accessor type. Topology accessors are defined by the interface of
  the underlying, user-defined type, i.e., unlike storage class accessors,
  topologies can be customized by the specialization to add types and inerfaces
  that are not part of the core FleCSI topology type. By inheriting from the
  customized topology type, we pick up these additions.
 */

template<typename TOPOLOGY_TYPE, size_t PRIVILEGES>
struct topology_accessor : public TOPOLOGY_TYPE,
                           public topology_reference<TOPOLOGY_TYPE> {

  topology_accessor(topology_reference<TOPOLOGY_TYPE> const & reference)
    : topology_reference<TOPOLOGY_TYPE>(reference) {}

}; // struct topology_accessor

} // namespace data
} // namespace flecsi
