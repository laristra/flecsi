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
  specializations for data clients and storage classes.
 */

#include <flecsi/data/common/storage_classes.hh>
#include <flecsi/data/legion/topologies.hh>

namespace flecsi {
namespace data {

struct legion_data_policy_t {

  /*--------------------------------------------------------------------------*
    Topology Instance Interface.
   *--------------------------------------------------------------------------*/

  template<typename TOPOLOGY_TYPE>
  using topology_instance_u = typename legion::topology_instance_u<
    typename TOPOLOGY_TYPE::type_identifier_t>;

  template<typename TOPOLOGY_TYPE>
  static void create(
    topology_reference_u<typename TOPOLOGY_TYPE::type_identifier_t> const &
      topology_reference,
    typename TOPOLOGY_TYPE::coloring_t const & coloring) {
    topology_instance_u<typename TOPOLOGY_TYPE::type_identifier_t>::create(
      topology_reference, coloring);
  } // create

  template<typename TOPOLOGY_TYPE>
  static void destroy(
    topology_reference_u<typename TOPOLOGY_TYPE::type_identifier_t> const &
      topology_reference) {
    topology_instance_u<typename TOPOLOGY_TYPE::type_identifier_t>::destroy(
      topology_reference);
  } // destroy

  /*--------------------------------------------------------------------------*
    Topology Accessor Interface.
   *--------------------------------------------------------------------------*/

  /*--------------------------------------------------------------------------*
    Field Accessor Interface.
   *--------------------------------------------------------------------------*/

  // FIXME: These can move up into common
  template<typename DATA_TYPE, size_t PRIVILEGES>
  using global_accessor_u =
    global_topology::accessor_u<DATA_TYPE, PRIVILEGES>;

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using index_accessor_u =
    index_topology::accessor_u<DATA_TYPE, PRIVILEGES>;

#if 0
  template<typename DATA_TYPE, size_t PRIVILEGES>
  using dense_unstructured_mesh_accessor_u =
    legion::unstructured_mesh_topology::dense_accessor_u<DATA_TYPE, PRIVILEGES>;

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using dense_structured_mesh_accessor_u =
    legion::structured_mesh_topology::dense_accessor_u<DATA_TYPE, PRIVILEGES>;
#endif

}; // struct legion_data_policy_t

} // namespace data
} // namespace flecsi
