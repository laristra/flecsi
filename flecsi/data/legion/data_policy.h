/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!
  @file

  This file is really just a conduit to capture the different
  specializations for data clients and storage classes.
 */

#include <flecsi/data/legion/client_handle_specializations.h>
#include <flecsi/data/legion/storage_classes.h>

namespace flecsi {
namespace data {

struct legion_data_policy_t {

  /*--------------------------------------------------------------------------*
    Client Interface.
   *--------------------------------------------------------------------------*/

  /*
    Documentation for this interface is in the top-level client type.
   */

  template<typename CLIENT_TYPE, size_t NAMESPACE, size_t NAME>
  static client_handle_u<CLIENT_TYPE, 0> get_client_handle() {
    using client_handle_specialization_t =
      legion::client_handle_specialization_u<
        typename CLIENT_TYPE::type_identifier_t>;

    return client_handle_specialization_t::template get_client_handle<NAMESPACE,
      NAME>();
  } // get_client_handle

  /*--------------------------------------------------------------------------*
    Storage Class Interface.
   *--------------------------------------------------------------------------*/

  /*
    Capture the base storage class type. This is necessary as a place
    holder in the field interface.
   */

  template<size_t STORAGE_CLASS, typename CLIENT_TYPE>
  using storage_class_u = legion::storage_class_u<STORAGE_CLASS, CLIENT_TYPE>;

  /*--------------------------------------------------------------------------*
    Accessor Interface.
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using global_accessor_u =
    legion::global_topology::accessor_u<DATA_TYPE, PRIVILEGES>;

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using color_accessor_u =
    legion::color_topology::accessor_u<DATA_TYPE, PRIVILEGES>;

}; // struct legion_data_policy_t

} // namespace data
} // namespace flecsi
