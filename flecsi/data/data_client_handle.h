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

#include <flecsi/data/common/data_reference.h>

namespace flecsi {

/*!
  This empty base class is the base of all accessors and is used by the
  handle tuple walkers for type checking.
 */

struct data_client_handle_base_t : public data::data_reference_base_t {};

/*!
  This class provides template parameters for data client type
  and permissions - either read or write - and the common fields
  such as necessary hashes used by various data client types such as
  set and mesh topology.
 */

template<typename DATA_CLIENT_TYPE, size_t PERMISSIONS, typename DATA_POLICY>
struct data_client_handle_base_u : public DATA_CLIENT_TYPE,
                                   public DATA_POLICY,
                                   public data_client_handle_base_t {
  using type = DATA_CLIENT_TYPE;

  data_client_handle_base_u() {}

  /*!
    This method is used to ensure that the data client handle is never
    constructed in calling a task with the unmapped (0) permissions.
   */

  template<size_t UNMAPPED_PERMISSIONS>
  data_client_handle_base_u(const data_client_handle_base_u<DATA_CLIENT_TYPE,
    UNMAPPED_PERMISSIONS,
    DATA_POLICY> & h)
    : DATA_CLIENT_TYPE(h), DATA_POLICY(h), type_hash(h.type_hash),
      name_hash(h.name_hash), namespace_hash(h.namespace_hash) {
    static_assert(
      UNMAPPED_PERMISSIONS == 0, "passing mapped client handle to task args");
  }

  size_t type_hash;
  size_t name_hash;
  size_t namespace_hash;
}; // struct data_client_handle_base_u

template<typename T>
struct data_client_type_u {};

template<typename DATA_CLIENT_TYPE, size_t PERMISSIONS, typename DATA_POLICY>
struct data_client_type_u<flecsi::
    data_client_handle_base_u<DATA_CLIENT_TYPE, PERMISSIONS, DATA_POLICY>> {
  using type = DATA_CLIENT_TYPE;
};

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_client_handle_policy.h>

namespace flecsi {

template<typename T, std::size_t Perms>
using data_client_handle =
  data_client_handle_base_u<T, Perms, FLECSI_RUNTIME_DATA_CLIENT_HANDLE_POLICY>;

/*!
  The data_handle_u type is the high-level data handle type.

  @tparam DATA_CLIENT_TYPE The client type.
  @tparam DATA_POLICY      The data policy for this handle type.

  @ingroup data
 */

template<typename DATA_CLIENT_TYPE, size_t PERMISSIONS>
using data_client_handle_u =
  data_client_handle<DATA_CLIENT_TYPE, PERMISSIONS> &;

} // namespace flecsi
