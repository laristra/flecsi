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

#ifndef flecsi_mpi_storage_policy_h
#define flecsi_mpi_storage_policy_h

//#include <cassert>
//#include <memory>
//#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
//#include <vector>

#include "flecsi/data/common/data_hash.h"
#include "flecsi/data/data_constants.h"
#include "flecsi/runtime/types.h"

// Include partial specializations
//#include "flecsi/data/mpi/global.h"
#include "flecsi/data/mpi/dense.h"
//#include "flecsi/data/mpi/sparse.h"
//#include "flecsi/data/mpi/scoped.h"
//#include "flecsi/data/mpi/tuple.h"

namespace flecsi {
namespace data {

struct mpi_storage_policy_t {

  using registration_function_t = std::function<void(size_t)>;
  using field_registration_function_t = std::function<void(size_t, size_t)>;
  using unique_fid_t = utils::unique_id_t<field_id_t, FLECSI_GENERATED_ID_MAX>;
  using field_value_t = std::pair<field_id_t, field_registration_function_t>;
  using client_value_t = std::pair<field_id_t, registration_function_t>;

  using field_entry_t = std::unordered_map<size_t, field_value_t>;
  using client_entry_t = std::unordered_map<size_t, client_value_t>;

  bool
  register_field(
    size_t client_key,
    size_t key,
    const field_registration_function_t & callback
  )
  {
    if(field_registry_.find(client_key) != field_registry_.end()) {
      if(field_registry_[client_key].find(key) !=
        field_registry_[client_key].end()) {
        clog(warn) << "field key already exists" << std::endl;
      } // if
    } // if

    field_registry_[client_key][key] =
      std::make_pair(unique_fid_t::instance().next(), callback);

    return true;
  }

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

  bool
  register_client_fields(size_t client_key)
  {
    return registered_client_fields_.insert(client_key).second;
  }

private:
  std::unordered_set<size_t> registered_client_fields_;
  std::unordered_map<size_t, field_entry_t> field_registry_;
  std::unordered_map<size_t, client_entry_t> client_registry_;

}; // struct mpi_storage_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_mpi_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
