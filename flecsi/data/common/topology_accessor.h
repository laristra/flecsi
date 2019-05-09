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
#include <flecsi/data/common/data_reference.h>
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
struct topology_accessor_u : public TOPOLOGY_TYPE,
  public data_reference_base_t {
  static constexpr size_t privileges = PRIVILEGES;
}; // struct topology_accessor_u

} // namespace data
} // namespace flecsi
