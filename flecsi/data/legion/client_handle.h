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
  @tparam PERMISSIONS The access permissions requested for this handle.
 */

template<typename CLIENT_TYPE, size_t PERMISSIONS>
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

  template<size_t UNMAPPED_PERMISSIONS>
  client_handle_u(const client_handle_u<CLIENT_TYPE, UNMAPPED_PERMISSIONS> & h)
    : CLIENT_TYPE(h), type_hash(h.type_hash), name_hash(h.name_hash),
      namespace_hash(h.namespace_hash) {
    static_assert(
      UNMAPPED_PERMISSIONS == 0, "passing mapped client handle to task args");
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
} // namespace flecsi
