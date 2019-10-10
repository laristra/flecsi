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

#include <flecsi/data/reference.hh>

namespace flecsi {
namespace data {

struct mpi_policy_t {
  /*--------------------------------------------------------------------------*
    Topology Instance Interface.
   *--------------------------------------------------------------------------*/

  template<typename TOPOLOGY_TYPE>
  static void allocate(reference_base const & topology_reference,
    reference_base const & coloring_reference) {
    // TBD
  } // allocate

  template<typename TOPOLOGY_TYPE>
  static void deallocate(reference_base const & topology_reference) {
    // TBD
  } // deallocate

}; // struct mpi_policy_t

} // namespace data
} // namespace flecsi
