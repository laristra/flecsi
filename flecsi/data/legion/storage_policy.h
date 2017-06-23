/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef FLECSI_DATA_LEGION_storage_policy_h
#define FLECSI_DATA_LEGION_storage_policy_h

#include <cinchlog.h>
#include <legion.h>
#include <functional>
#include <unordered_map>

#include "flecsi/utils/common.h"

//----------------------------------------------------------------------------//
// @file
// @date Initial file creation: Apr 17, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {

struct legion_storage_policy_t {

  //--------------------------------------------------------------------------//
  // Public type definitions.
  //--------------------------------------------------------------------------//

  using field_id_t = Legion::FieldID;
  using registration_function_t = std::function<void(size_t)>;
  using unique_fid_t = utils::unique_id_t<field_id_t, FLECSI_GENERATED_ID_MAX>;
  using data_value_t = std::pair<field_id_t, registration_function_t>;

  using field_entry_t = std::unordered_map<size_t, data_value_t>;

  // Field and client registration interfaces are the same for now.
  using client_entry_t = field_entry_t;

  //--------------------------------------------------------------------------//
  //! Register a field with the runtime.
  //!
  //! @param client_key The data client indentifier hash.
  //! @param key        The identifier hash.
  //! @param callback   The registration call back function.
  //--------------------------------------------------------------------------//

  bool
  register_field(
    size_t client_key,
    size_t key,
    const registration_function_t & callback
  )
  {
    if(field_registry_.find(client_key) != field_registry_.end()) {
      clog_assert(field_registry_[client_key].find(key) ==
        field_registry_[client_key].end(),
        "field key already exists");
    } // if

    field_registry_[client_key][key] =
      std::make_pair(unique_fid_t::instance().next(), callback);

    return true;
  } // register_field

////////////////////////////////////////////////////////////////////////////////
// FIXME: These probably need to be moved up a level
  void
  register_all()
  {
    for(auto & c: field_registry_) {
      for(auto & d: c.second) {
        d.second.second(d.second.first);
      } // for
    } // for
  } // register_all

  auto const &
  field_registry()
  const
  {
    return field_registry_;
  } // field_registry

  auto const &
  client_registry()
  const
  {
    return client_registry_;
  } // client_registry

////////////////////////////////////////////////////////////////////////////////

  //--------------------------------------------------------------------------//
  //! Register a client with the runtime.
  //!
  //! @param client_key The data client indentifier hash.
  //! @param key        The identifier hash.
  //! @param callback   The registration call back function.
  //--------------------------------------------------------------------------//

  bool
  register_client(
    size_t client_key,
    size_t key,
    const registration_function_t & callback
  )
  {
    if(client_registry_.find(client_key) != client_registry_.end()) {
      clog_assert(client_registry_[client_key].find(key) ==
        client_registry_[client_key].end(),
        "client key already exists");
    } // if

    client_registry_[client_key][key] =
      std::make_pair(unique_fid_t::instance().next(), callback);

    return true;
  } // register_client

private:

  std::unordered_map<size_t, field_entry_t> field_registry_;
  std::unordered_map<size_t, client_entry_t> client_registry_;

}; // struct legion_storage_policy_t

} // namespace data
} // namespace flecsi

#endif // FLECSI_DATA_LEGION_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
