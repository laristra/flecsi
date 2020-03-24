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
#include <flecsi/topology/core.hh>

#include <optional>

namespace flecsi {
namespace data {

// Clients #include these (backend-specific) definitions elsewhere:

// CRTP base for registering topology slots with the context if needed.
template<class>
struct topology_id;
template<typename>
struct topology_data;

template<typename TOPOLOGY_TYPE>
struct topology_slot : topology_id<topology_slot<TOPOLOGY_TYPE>> {

  using core_t = topology::core_t<TOPOLOGY_TYPE>;
  static_assert(sizeof(TOPOLOGY_TYPE) == sizeof(core_t),
    "topologies may not add data members");
  using data_t = topology_data<topology::category_t<core_t>>;

  using coloring = typename TOPOLOGY_TYPE::coloring;

  data_t & allocate(const coloring & coloring_reference) {
    return data.emplace(coloring_reference);
  } // allocate

  void deallocate() {
    //data.reset();
  } // deallocate

  data_t & get() {
    return *data;
  }
  const data_t & get() const {
    return *data;
  }

private:
  static const bool static_registered_;
  // Force instantiation, working around GCC 9.1/9.2 bug #92062 with an
  // explicitly dependent condition:
  static_assert(((void)&static_registered_, sizeof(TOPOLOGY_TYPE)));

  std::optional<data_t> data;
}; // struct topology_slot

template<typename TOPOLOGY_TYPE>
const bool topology_slot<TOPOLOGY_TYPE>::static_registered_ =
  (topology_registration<TOPOLOGY_TYPE>::register_fields(), true);

} // namespace data
} // namespace flecsi
