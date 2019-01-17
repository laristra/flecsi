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

namespace flecsi {

/*!
  This empty base class is the base of all data client types, and is used
  by the handle tuple walkers for type checking.
 */

struct client_handle_base_t {};

/*!
  This type bundles client handle information that is common to all
  client types.

  @tparam CLIENT_TYPE The data client type.
  @tparam PERMISSIONS The access permissions requested for this handle.
  @tparam DATA_POLICY The runtime-specific data policy.
 */

template<typename CLIENT_TYPE, size_t PERMISSIONS, typename DATA_POLICY>
struct client_handle_base_u : public CLIENT_TYPE, public DATA_POLICY, public client_handle_base_t {
  using type = CLIENT_TYPE;

  /*!
    Empty constructor.
   */

  client_handle_base_u() {}

  /*!
    This constructor ensures that a data client handle is never
    passed to a task with the unmapped (0) permissions.
   */

  template<size_t UNMAPPED_PERMISSIONS>
  client_handle_base_u(const client_handle_base_u<CLIENT_TYPE,
    UNMAPPED_PERMISSIONS,
    DATA_POLICY> & h)
    : DATA_POLICY(h), CLIENT_TYPE(h), type_hash(h.type_hash),
      name_hash(h.name_hash), namespace_hash(h.namespace_hash) {
    static_assert(
      UNMAPPED_PERMISSIONS == 0, "passing mapped client handle to task args");
  }

  /*!
    Copy constructor.
   */

  client_handle_base_u(const client_handle_base_u & h)
    : DATA_POLICY(h), CLIENT_TYPE(h), type_hash(h.type_hash),
      name_hash(h.name_hash), namespace_hash(h.namespace_hash) {}

  size_t type_hash;
  size_t name_hash;
  size_t namespace_hash;
}; // struct client_handle_base_u

template<typename T>
struct client_type_u {};

template<typename CLIENT_TYPE, size_t PERMISSIONS, typename DATA_POLICY>
struct client_type_u<flecsi::
    client_handle_base_u<CLIENT_TYPE, PERMISSIONS, DATA_POLICY>> {
  using type = CLIENT_TYPE;
};

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_client_handle_policy.h>

namespace flecsi {

/*!
  The data_handle_u type is the high-level data handle type.

  @tparam CLIENT_TYPE The client type.
  @tparam DATA_POLICY The data policy for this handle type.

  @ingroup data
 */

template<typename CLIENT_TYPE, size_t PERMISSIONS>
using client_handle_u = client_handle_base_u<CLIENT_TYPE,
  PERMISSIONS,
  FLECSI_RUNTIME_CLIENT_HANDLE_POLICY>;

} // namespace flecsi
