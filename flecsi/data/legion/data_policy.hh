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

#include <flecsi/data/legion/topologies.hh>

namespace flecsi {
namespace data {

struct legion_data_policy_t {

  /*--------------------------------------------------------------------------*
    Topology Instance Interface.
   *--------------------------------------------------------------------------*/

  template<typename TOPOLOGY_TYPE>
  static void create(
    topology_reference<TOPOLOGY_TYPE> const & topology_reference,
    typename TOPOLOGY_TYPE::coloring_t const & coloring) {
    legion::topology_instance<TOPOLOGY_TYPE>::create(
      topology_reference, coloring);
  } // create

  template<typename TOPOLOGY_TYPE>
  static void destroy(
    topology_reference<TOPOLOGY_TYPE> const & topology_reference) {
    legion::topology_instance<TOPOLOGY_TYPE>::destroy(topology_reference);
  } // destroy

  /*--------------------------------------------------------------------------*
    Topology Accessor Interface.
   *--------------------------------------------------------------------------*/

}; // struct legion_data_policy_t

} // namespace data
} // namespace flecsi
