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

#ifndef flecsi_serial_storage_policy_h
#define flecsi_serial_storage_policy_h

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeinfo>
#include <cassert>

#include "flecsi/data/common/data_hash.h"

#include "flecsi/data/data_constants.h"
#include "flecsi/data/serial/meta_data.h"
#include "flecsi/data/serial/registration_wrapper.h"

// Include partial specializations
//#include "flecsi/data/serial/global.h"
#include "flecsi/data/serial/dense.h"
//#include "flecsi/data/serial/sparse.h"
//#include "flecsi/data/serial/scoped.h"
//#include "flecsi/data/serial/tuple.h"

///
/// \file
/// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {

struct serial_storage_policy_t {

  // Define the storage type
  template<size_t data_type_t>
  using storage_type_t = serial::storage_type_t<data_type_t>;

  using field_id_t = size_t;
  using registration_function_t = std::function<void(size_t)>;

  using data_value_t = std::pair<field_id_t, registration_function_t>;

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
    serial_registration_wrapper__<
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
    serial_client_registration_wrapper__<
      DATA_CLIENT_TYPE,
      NAMESPACE_HASH,
      NAME_HASH>;

  /// \brief delete ALL data.
  void
  reset()
  {
    assert(false && "unimplemented");
  } // reset

  ///
  /// \brief delete ALL data associated with this runtime namespace.
  /// \param [in] runtime_namespace the namespace to search.
  ///
  void
  reset(
    uintptr_t runtime_namespace
  )
  {
    assert(false && "unimplemented");
  } // reset

  /// \brief Count all data associated with this runtime namespace.
  /// \param [in] runtime_namespace the namespace to search.
  /// \return The number of hits for this namespace.
  size_t
  count(
    uintptr_t runtime_namespace
  )
  {
    assert(false && "unimplemented");
  } // count

  /// \brief move ALL data associated with this runtime namespace
  /// \param [in] from,to the namespaces to move data from and to.
  void move( uintptr_t from, uintptr_t to ) {
    assert(false && "unimplemented");
  } // move

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE,
    size_t NAME
  >
  decltype(auto)
  get_client_handle()
  {
    data_client_handle__<DATA_CLIENT_TYPE> client_handle;
    return client_handle;
  } // get_client_handle

}; // struct serial_storage_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_serial_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
