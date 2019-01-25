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

namespace flecsi {
namespace data {

struct legion_data_policy_t {

  /*
    Documnetation for this interface is in the top-level client type.
   */

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static client_handle_u<DATA_CLIENT_TYPE, 0> get_client_handle() {

    using client_handler_t =
      client_handler_u<typename DATA_CLIENT_TYPE::type_identifier>;

    return client_handler_t::template get_client_handle<DATA_CLIENT_TYPE,
      NAMESPACE_HASH, NAME_HASH>();

  } // get_client_handle

}; // struct legion_data_policy_t

} // namespace data
} // namespace flecsi
