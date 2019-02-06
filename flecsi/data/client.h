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

  This file contains the C++ data model interface \em client_interface_t
  for FleCSI topology data structures, which are also referred to as
  \em data \em clients in this context. The \em client_interface_t type is
  a specialization of the \em client_interface_u type on the backend
  runtime policy that is selected at compile time.
 */

#include <flecsi/data/common/client_registration_wrapper.h>
#include <flecsi/execution/context.h>
#include <flecsi/utils/flog.h>
#include <flecsi/utils/hash.h>

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

    @tparam CLIENT_TYPE    The data client type.
    @tparam NAMESPACE_HASH The namespace key. Namespaces allow separation
                           of attribute names to avoid collisions.
    @tparam NAME_HASH      The attribute name.
   */

  template<typename CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static bool register_client(std::string const & name) {
    static_assert(sizeof(CLIENT_TYPE) ==
                    sizeof(typename CLIENT_TYPE::type_identifier_t),
      "Data clients may not add data members");

    using wrapper_t = client_registration_wrapper_u<
      typename CLIENT_TYPE::type_identifier_t, NAMESPACE_HASH, NAME_HASH>;

    const size_t type_key =
      typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

    const size_t key = utils::hash::client_hash<NAMESPACE_HASH, NAME_HASH>();

    flog(internal)
      << "Registering data client" << std::endl
      << "\tname: " << name << std::endl
      << "\ttype: "
      << utils::demangle(
           typeid(typename CLIENT_TYPE::type_identifier_t).name())
      << std::endl;

    if(!execution::context_t::instance().register_client(
         type_key, key, wrapper_t::register_callback)) {
      return false;
    } // if

    return true;
  } // register_client

  /*!
    Return a handle to a data client.

    @tparam CLIENT_TYPE    The data client type.
    @tparam NAMESPACE_HASH The namespace key. Namespaces allow separation
                           of attribute names to avoid collisions.
    @tparam NAME_HASH      The attribute name.
   */

  template<typename CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static decltype(auto) get_client_handle() {
    return DATA_POLICY::template get_client_handle<CLIENT_TYPE,
      NAMESPACE_HASH, NAME_HASH>();
  } // get_client_handle

}; // struct client_interface_u

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/data_policy.h>

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
