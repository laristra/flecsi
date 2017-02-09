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

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeinfo>
#include <cassert>

#include "flecsi/data/data_constants.h"

#include "flecsi/data/legion/meta_data.h"

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
  // This will likely be much more complicated in a real policy
  using data_store_t = std::unordered_map<size_t,
    std::unordered_map<utils::const_string_t::hash_type_t, meta_data_t>>;

  // Define the storage type
  template<size_t data_type_t>
  using storage_type_t = legion::storage_type_t<data_type_t,
    data_store_t, meta_data_t>;

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
