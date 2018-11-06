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

#include <cinchlog.h>
#include <functional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <flecsi/runtime/types.h>
#include <flecsi/utils/common.h>

namespace flecsi {
namespace data {

/*!
  The storage_u type provides a high-level data model context interface that
  is implemented by the given storage policy. It provides an interface for
  client and field registration.

  @tparam USER_META_DATA A user-defined meta data type.
  @tparam STORAGE_POLICY The backend storage policy.

  @ingroup data
 */

template<typename STORAGE_POLICY>
struct storage_u : public STORAGE_POLICY {

  //--------------------------------------------------------------------------//
  // Public type definitions.
  //--------------------------------------------------------------------------//

  using registration_function_t = std::function<void(size_t)>;
  using field_registration_function_t = std::function<void(size_t, size_t)>;
  using unique_fid_t = utils::unique_id_t<field_id_t, FLECSI_GENERATED_ID_MAX>;
  using field_value_t = std::pair<field_id_t, field_registration_function_t>;
  using client_value_t = std::pair<field_id_t, registration_function_t>;

  using field_entry_t = std::unordered_map<size_t, field_value_t>;
  using client_entry_t = std::unordered_map<size_t, client_value_t>;

  /*!
    Register a field with the runtime.

    @param client_type_key The data client indentifier hash.
    @param key             The identifier hash.
    @param callback        The registration call back function.
   */

  bool register_field(size_t client_type_key,
    size_t key,
    const field_registration_function_t & callback) {
    if(field_registry_.find(client_type_key) != field_registry_.end()) {
      if(field_registry_[client_type_key].find(key) !=
         field_registry_[client_type_key].end()) {
        clog(warn) << "field key already exists" << std::endl;
      } // if
    } // if

    field_registry_[client_type_key][key] =
      std::make_pair(unique_fid_t::instance().next(), callback);

    return true;
  } // register_field

  /*!
   */

  auto const & field_registry() const {
    return field_registry_;
  } // field_registry

  /*!
   */

  auto const & client_registry() const {
    return client_registry_;
  } // client_registry

  /*!
    Register a client with the runtime.

    @param type_hash  The data client indentifier hash.
    @param key        The identifier hash.
    @param callback   The registration call back function.
   */

  bool register_client(size_t type_hash,
    size_t key,
    const registration_function_t & callback) {
    if(client_registry_.find(type_hash) != client_registry_.end()) {
      clog_assert(client_registry_[type_hash].find(key) ==
                    client_registry_[type_hash].end(),
        "client key already exists");
    } // if

    client_registry_[type_hash][key] =
      std::make_pair(unique_fid_t::instance().next(), callback);

    return true;
  } // register_client

  /*!
    Return a boolean indicating whether or not the given instance of
    a data client has had its internal fields registered with the
    data model.

    @param type_key     The hash key for the data client type.
    @param instance_key The hash key for the data client instance.
   */

  bool register_client_fields(size_t type_key, size_t instance_key) {
    return registered_client_fields_
      .insert(std::make_pair(type_key, instance_key))
      .second;
  } // register_client_fields

  /*!
    Search for a client at the runtime.

    @param client_key The data client indentifier hash.
   */

  void assert_client_exists(size_t type_hash, size_t client_hash) {
    clog_assert(client_registry_.find(type_hash) != client_registry_.end(),
      "\nThe data_client type you are trying to access with key "
        << type_hash << " does not exist!"
        << "\nMake sure it has been properly registered!");
    clog_assert(client_registry_[type_hash].find(client_hash) !=
                  client_registry_[type_hash].end(),
      "\nThe data_client instance you are trying to access with key "
        << client_hash << " does not exist!"
        << "\nMake sure it has been properly registered!");
  } // register_client

  /*!
    Myer's singleton instance.

    @return The single instance of this type.
   */

  static storage_u & instance() {
    static storage_u d;
    return d;
  } // instance

  //--------------------------------------------------------------------------//
  // FIXME: What are these for?
  //--------------------------------------------------------------------------//

  void move(uintptr_t from, uintptr_t to) {}

  void reset(uintptr_t runtime_namespace) {}

private:
  // Default constructor
  storage_u() : STORAGE_POLICY() {}

  // Destructor
  ~storage_u() {}

  // We don't need any of these
  storage_u(const storage_u &) = delete;
  storage_u & operator=(const storage_u &) = delete;
  storage_u(storage_u &&) = delete;
  storage_u & operator=(storage_u &&) = delete;

  std::set<std::pair<size_t, size_t>> registered_client_fields_;
  std::unordered_map<size_t, field_entry_t> field_registry_;
  std::unordered_map<size_t, client_entry_t> client_registry_;

}; // class storage_u

} // namespace data
} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_storage_policy.h>

namespace flecsi {
namespace data {

using storage_t = storage_u<FLECSI_RUNTIME_STORAGE_POLICY>;

} // namespace data
} // namespace flecsi
