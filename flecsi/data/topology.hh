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
#include <flecsi/data/reference.hh>
#include <flecsi/data/topology_registration.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/topology/core.hh>
#include <flecsi/utils/flog.hh>

namespace flecsi {
namespace data {

template<typename TOPOLOGY_TYPE>
struct topology_reference : public reference_base {

  using core_t = topology::core_t<TOPOLOGY_TYPE>;
  static_assert(sizeof(TOPOLOGY_TYPE) == sizeof(core_t),
    "topologies may not add data members");

  using traits = topology_traits<core_t>;

  using coloring = typename TOPOLOGY_TYPE::coloring;

  topology_reference() : reference_base(unique_tid_t::instance().next()) {}

  ~topology_reference() {
    if(allocated_) {
      traits::deallocate(identifier_);
    } // if
  }

  void allocate(const coloring & coloring_reference) {
    traits::allocate(identifier_, coloring_reference);
    allocated_ = true;
  } // allocate

  void deallocate() {
    traits::deallocate(identifier_);
    allocated_ = false;
  } // deallocate

private:
  static const bool static_registered_;
  // Force instantiation, working around GCC 9.1/9.2 bug #92062 with an
  // explicitly dependent condition:
  static_assert(((void)&static_registered_, sizeof(TOPOLOGY_TYPE)));

  bool allocated_ = false;

}; // struct topology_reference

template<typename TOPOLOGY_TYPE>
const bool topology_reference<TOPOLOGY_TYPE>::static_registered_ =
  topology_registration<TOPOLOGY_TYPE>::register_fields();

} // namespace data
} // namespace flecsi
