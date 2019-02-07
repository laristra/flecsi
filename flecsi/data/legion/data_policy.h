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

/*! @file */

#include <flecsi/data/legion/client_handlers.h>

#include <flecsi/data/legion/dense.h>

namespace flecsi {
namespace data {

struct legion_data_policy_t {

  template<size_t STORAGE_CLASS, typename CLIENT_TYPE>
  using storage_class_u = legion::storage_class_u<STORAGE_CLASS, CLIENT_TYPE>;

  /*
    Documnetation for this interface is in the top-level client type.
   */

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static client_handle_u<DATA_CLIENT_TYPE, 0> get_client_handle() {
    using client_handler_t =
      legion::client_handler_u<typename DATA_CLIENT_TYPE::type_identifier>;

    return client_handler_t::template get_client_handle<NAMESPACE_HASH,
      NAME_HASH>();
  } // get_client_handle

}; // struct legion_data_policy_t

} // namespace data
} // namespace flecsi
