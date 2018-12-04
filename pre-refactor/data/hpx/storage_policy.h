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

#ifndef flecsi_hpx_storage_policy_h
#define flecsi_hpx_storage_policy_h

#include <cassert>
#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "flecsi/data/common/data_hash.h"

#include "flecsi/data/data_constants.h"
#include "flecsi/data/hpx/meta_data.h"
#include "flecsi/data/hpx/registration_wrapper.h"

// Include partial specializations
//#include "flecsi/data/hpx/global.h"
#include "flecsi/data/hpx/dense.h"
//#include "flecsi/data/hpx/sparse.h"
//#include "flecsi/data/hpx/scoped.h"
//#include "flecsi/data/hpx/tuple.h"

///
/// \file
/// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {

struct hpx_storage_policy_t {

  using field_id_t = size_t;
  using registration_function_t = std::function<void(size_t)>;
  using data_value_t = std::pair<field_id_t, registration_function_t>;
  using field_entry_t = std::unordered_map<size_t, data_value_t>;

  // Field and client registration interfaces are the same for now.
  using client_entry_t = field_entry_t;

  bool register_field(
      size_t client_key,
      size_t key,
      const registration_function_t & callback) {
    // TODO:
    return true;
  }

  void register_all() {
    for (auto & c : field_registry_) {
      for (auto & d : c.second) {
        d.second.second(d.second.first);
      } // for
    } // for
  } // register_all

  auto const & field_registry() const {
    return field_registry_;
  } // field_registry

  auto const & client_registry() const {
    return client_registry_;
  } // client_registry

private:
  std::unordered_map<size_t, field_entry_t> field_registry_;
  std::unordered_map<size_t, client_entry_t> client_registry_;
}; // struct hpx_storage_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_hpx_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
