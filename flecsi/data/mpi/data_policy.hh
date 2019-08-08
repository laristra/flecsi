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

#include <flecsi/data/mpi/client_handle_specializations.hh>
#include <flecsi/data/mpi/storage_classes.hh>

namespace flecsi {
namespace data {

struct mpi_data_policy_t {

  /*--------------------------------------------------------------------------*
    Client Interface.
   *--------------------------------------------------------------------------*/

  /*
    Documnetation for this interface is in the top-level client type.
   */

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE, size_t NAME>
  static client_handle<DATA_CLIENT_TYPE, 0> get_client_handle() {
    using client_handle_specialization_t =
      mpi::client_handle_specialization<DATA_CLIENT_TYPE>;

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
  using storage_class = mpi::storage_class<STORAGE_CLASS, CLIENT_TYPE>;

}; // struct mpi_data_policy_t

} // namespace data
} // namespace flecsi
