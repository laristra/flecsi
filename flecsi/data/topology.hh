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

#include <flecsi/data/backend.hh>
#include <flecsi/data/coloring.hh>
#include <flecsi/data/data_reference.hh>
#include <flecsi/data/topology_registration.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/topology/core.hh>
#include <flecsi/utils/flog.hh>

namespace flecsi {
namespace data {

template<typename TOPOLOGY_TYPE>
struct topology_reference : public data_reference_base_t {

  using core_t = topology::core_t<TOPOLOGY_TYPE>;
  static_assert(sizeof(TOPOLOGY_TYPE) == sizeof(core_t),
    "topologies may not add data members");

  topology_reference()
    : data_reference_base_t(unique_tid_t::instance().next()) {}

  void create(coloring_reference<TOPOLOGY_TYPE> const & coloring_reference) {
    data_policy_t::create(identifier_, coloring_reference);
  } // create

private:
  static bool static_registered_;

  // The static_registered_ variable must be referenced so that
  // register_fields() is called at most once.
  const bool registered_ = static_registered_;

}; // struct topology_reference

template<typename TOPOLOGY_TYPE>
bool topology_reference<TOPOLOGY_TYPE>::static_registered_ =
topology_registration<TOPOLOGY_TYPE>::register_fields();

} // namespace data
} // namespace flecsi
