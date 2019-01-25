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

#include <string>

namespace flecsi {
namespace data {

/*!
  The client_interface_u type defines a high-level data client
  interface that is implemented by the given data policy.

  @tparam DATA_POLICY The backend runtime policy.

  @ingroup data
 */

template<typename DATA_POLICY>
struct client_interface_u {

  /*!
    Register a data client with the FleCSI runtime.

    @tparam DATA_CLIENT_TYPE The data client type.
    @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
                             of attribute names to avoid collisions.
    @tparam NAME_HASH        The attribute name.
   */

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static bool register_client(std::string const & name) {
    return true;
  } // register_client

  /*!
    Return a handle to a data client.

    @tparam DATA_CLIENT_TYPE The data client type.
    @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
                             of attribute names to avoid collisions.
    @tparam NAME_HASH        The attribute name.
   */

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static decltype(auto) get_client_handle() {
    return DATA_POLICY::template get_client_handle<DATA_CLIENT_TYPE,
      NAMESPACE_HASH, NAME_HASH>();
  } // get_client_handle

}; // struct client_interface_u

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/flecsi_runtime_data_policy.h>

namespace flecsi {
namespace data {

/*!
  The client_interface_t type is the high-level interface to the FleCSI
  data client model.

  @ingroup data
 */

using client_interface_t = client_interface_u<FLECSI_RUNTIME_DATA_POLICY>;

} // namespace data
} // namespace flecsi
