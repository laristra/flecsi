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

  This file defines the type identifier type \em client_handle_base_t, and
  the basic client handle type \em client_handle_u, which is parameterized
  on the \em data \em client type. These types provide the basic structure
  for implementing handles to the various FleCSI data client types.

  A handle is a cross-address-space-safe reference type (like a pointer)
  that can be passed into a FleCSI task to allow access to a data client.
  Data clients are types that expose one or more index spaces that can
  have fields registered against them. Some examples are
  \ref mesh_topology_u, and \ref tree_topology_u.
 */

#include <cstddef>

namespace flecsi {
namespace data {

/*!
  This empty base class is the base of all data client types, and is used
  by the handle tuple walkers for type checking.
 */

struct client_handle_base_t {};

/*!
  This type bundles client handle information that is common to all
  client types.

  @tparam CLIENT_TYPE The data client type.
  @tparam PRIVILEGES  The access permissions requested for this handle.
 */

template<typename CLIENT_TYPE, size_t PRIVILEGES>
struct client_handle_u : public CLIENT_TYPE, public client_handle_base_t {

  using type = CLIENT_TYPE;

  /*!
    Empty constructor.
   */

  client_handle_u() {}

  /*!
    This constructor ensures that a data client handle is never
    passed to a task with the unmapped (0) permissions.
   */

  template<size_t UNMAPPED_PRIVILEGES>
  client_handle_u(const client_handle_u<CLIENT_TYPE, UNMAPPED_PRIVILEGES> & h)
    : CLIENT_TYPE(h), type_hash(h.type_hash), name_hash(h.name_hash),
      namespace_hash(h.namespace_hash) {
    static_assert(
      UNMAPPED_PRIVILEGES == 0, "passing mapped client handle to task args");
  }

  /*!
    Copy constructor.
   */

  client_handle_u(const client_handle_u & h)
    : CLIENT_TYPE(h), type_hash(h.type_hash), name_hash(h.name_hash),
      namespace_hash(h.namespace_hash) {}

  /*
    Public data members.
   */

  size_t type_hash;
  size_t name_hash;
  size_t namespace_hash;

}; // struct client_handle_u

} // namespace data

/*!
  Base type to recover client type from a handle.
 */

template<typename T>
struct client_type_u {};

/*!
  Recover type from handle.
 */

template<typename CLIENT_TYPE, size_t PRIVILEGES>
struct client_type_u<data::client_handle_u<CLIENT_TYPE, PRIVILEGES>> {
  using type = CLIENT_TYPE;
};

} // namespace flecsi
