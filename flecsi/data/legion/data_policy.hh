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

  This file is really just a conduit to capture the different
  specializations for accessors and topologies.
 */

#include <flecsi/data/legion/topology_traits.hh>

namespace flecsi {
namespace data {

struct legion_data_policy_t {

  /*--------------------------------------------------------------------------*
    Topology Instance Interface.
   *--------------------------------------------------------------------------*/

  template<typename TOPOLOGY_TYPE>
  static void create(
    data_reference_base_t const & topology_reference,
    data_reference_base_t const & coloring_reference) {
    legion::topology_traits<TOPOLOGY_TYPE>::create(
      topology_reference, coloring_reference);
  } // create

  template<typename TOPOLOGY_TYPE>
  static void destroy(
    data_reference_base_t const & topology_reference) {
    legion::topology_traits<TOPOLOGY_TYPE>::destroy(topology_reference);
  } // destroy

}; // struct legion_data_policy_t

} // namespace data
} // namespace flecsi
