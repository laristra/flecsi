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

#include "flecsi/data/data_constants.h"

#include "flecsi/data/serial/meta_data.h"

// Include partial specializations
#include "flecsi/data/serial/global.h"
#include "flecsi/data/serial/dense.h"
#include "flecsi/data/serial/sparse.h"
#include "flecsi/data/serial/scoped.h"
#include "flecsi/data/serial/tuple.h"

/*!
 * \file serial/storage_policy.h
 * \authors bergen
 * \date Initial file creation: Apr 17, 2016
 */

namespace flecsi {
namespace data {

template<typename user_meta_data_t>
struct serial_storage_policy_t {

  using meta_data_t = serial_meta_data_t<user_meta_data_t>;

  // Define the data store type
  // This will likely be much more complicated in a real policy
  using data_store_t = std::unordered_map<size_t,
    std::unordered_map<const_string_t::hash_type_t, meta_data_t>>;

  // Define the storage type
  template<size_t data_type_t>
  using storage_type_t = serial_storage_policy::storage_type_t<data_type_t,
    data_store_t, meta_data_t>;

  //! \brief delete ALL data.
  void
  reset()
  {
    data_store_.clear();
  } // reset

  /*! 
   * \brief delete ALL data associated with this runtime namespace.
   * \param [in] runtime_namespace the namespace to search.
   */
  void
  reset(
    uintptr_t runtime_namespace
  )
  {
    // check each namespace
    for (auto & sub_map : data_store_) {

      // the namespace data
      auto & namespace_key = sub_map.first;
      auto & meta_data = sub_map.second;
      
      // loop over each element in the namespace
      auto itr = meta_data.begin();
      while (itr != meta_data.end()) {
        // get the meta data key and label
        auto & meta_data_key = itr->first;
        auto & label = itr->second.label;
        // now build the hash for this label
        auto key_hash = hash<const_string_t::hash_type_t>(label, label.size());
        auto hash = key_hash ^ runtime_namespace;
        // test if it should be deleted
        if (meta_data_key == hash)
          itr = meta_data.erase(itr);
        else 
          ++itr;
      } // while
    } // for
  } // reset

protected:

  // Storage container instance
  data_store_t data_store_;

}; // struct serial_storage_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_serial_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
