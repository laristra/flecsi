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

#include <flecsi/data/data_reference.hh>

namespace flecsi {
namespace data {

struct mpi_data_policy_t {
  /*--------------------------------------------------------------------------*
    Topology Instance Interface.
   *--------------------------------------------------------------------------*/

  template<typename TOPOLOGY_TYPE, typename... ARGS>
  static void allocate_coloring(
    data_reference_base_t const & coloring_reference,
    ARGS &&... args) {
    // TBD
  } // allocate_coloring

  template<typename TOPOLOGY_TYPE>
  static void deallocate_coloring(
    data_reference_base_t const & coloring_reference) {
    // TBD
  } // deallocate_coloring

  template<typename TOPOLOGY_TYPE>
  static void allocate(data_reference_base_t const & topology_reference,
    data_reference_base_t const & coloring_reference) {
    // TBD
  } // allocate

  template<typename TOPOLOGY_TYPE>
  static void deallocate(data_reference_base_t const & topology_reference) {
    // TBD
  } // deallocate

}; // struct mpi_data_policy_t

} // namespace data
} // namespace flecsi
