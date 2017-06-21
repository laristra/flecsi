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

#ifndef flecsi_legion_storage_policy_h
#define flecsi_legion_storage_policy_h

#include <cassert>
#include <legion.h>
#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "flecsi/data/common/data_hash.h"
#include "flecsi/data/data_constants.h"
#include "flecsi/data/legion/meta_data.h"
#include "flecsi/data/legion/registration_wrapper.h"

// Include partial specializations
#include "flecsi/data/legion/global.h"
#include "flecsi/data/legion/dense.h"
#include "flecsi/data/legion/sparse.h"
#include "flecsi/data/legion/scoped.h"
#include "flecsi/data/legion/tuple.h"

///
// \file legion/storage_policy.h
// \authors bergen
// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {

template<typename user_meta_data_t>
struct legion_storage_policy_t {

  using meta_data_t = legion_meta_data_t<user_meta_data_t>;

  // Define the data store type
  // FIXME: THIS NEEDS TO BE IMPLEMENTED!!!
  using data_store_t = size_t;

  // Define the storage type
  template<size_t data_type_t>
  using storage_type_t = legion::storage_type_t<data_type_t,
    data_store_t, meta_data_t>;

  using field_id_t = Legion::FieldID;
  using registration_function_t = std::function<void(size_t)>;
  using unique_fid_t =
    utils::unique_id_t<field_id_t, FLECSI_GENERATED_ID_MAX>;

  using data_value_t = std::pair<field_id_t, registration_function_t>;

  using client_value_t =
    std::unordered_map<
      data_hash_t::key_t, // key
      data_value_t,       // value
      data_hash_t,        // hash function
      data_hash_t         // equivalence operator
    >;

  template<
    typename DATA_CLIENT_TYPE,
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t INDEX_SPACE,
    size_t VERSIONS
  >
  using registration_wrapper__ =
    legion_registration_wrapper__<
      DATA_CLIENT_TYPE,
      STORAGE_TYPE,
      DATA_TYPE,
      NAMESPACE_HASH,
      NAME_HASH,
      INDEX_SPACE,
      VERSIONS>;

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  using client_registration_wrapper__ =
    legion_client_registration_wrapper__<
      DATA_CLIENT_TYPE,
      NAMESPACE_HASH,
      NAME_HASH>;

  ///
  // \brief delete ALL data.
  ///
  void
  reset()
  {
  } // reset

  ///
  // \brief delete ALL data associated with this runtime namespace.
  // \param [in] runtime_namespace the namespace to search.
  ///
  void
  reset(
    uintptr_t runtime_namespace
  )
  {
  } // reset

  void move( uintptr_t from, uintptr_t to ) {
    assert(false && "unimplemented");
  }

  size_t
  count(
    uintptr_t runtime_namespace
  )
  {

  }

protected:

  // Storage container instance
  data_store_t data_store_;

}; // struct legion_storage_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_legion_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
