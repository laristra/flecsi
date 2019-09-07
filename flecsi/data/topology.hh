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

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <flecsi/data/topology_registration.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/topology/core.hh>
#include <flecsi/utils/flog.hh>

namespace flecsi {
namespace data {

template<typename TOPOLOGY_TYPE>
struct topology_need_name {

  using core_t = topology::core_t<TOPOLOGY_TYPE>;
  static_assert(sizeof(TOPOLOGY_TYPE) == sizeof(core_t),
    "topologies may not add data members");

  topology_need_name() {
    topology_registration<TOPOLOGY_TYPE>::register_fields();
  } // topology

  topology_reference<core_t> operator()() const {
    return {unique_tid_t::instance().next()};
  }

}; // struct topology_need_name

} // namespace data
} // namespace flecsi
