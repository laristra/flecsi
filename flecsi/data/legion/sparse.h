/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <algorithm>
#include <map>

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_class.h!!!
// Using this approach allows us to have only one storage_class__
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include "flecsi/data/storage_class.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/utils/const_string.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 17, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {
namespace legion {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Helper type definitions.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Sparse accessor.
//----------------------------------------------------------------------------//

template<typename T>
struct sparse_mutator_t {

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  sparse_mutator_t() {}

  ///
  //
  ///
  sparse_mutator_t(
      size_t num_slots,
      const std::string & label,
      size_t version) {}

  ///
  //
  ///
  ~sparse_mutator_t() {} // ~sparse_mutator_t

  ///
  //
  ///
  T & operator()(size_t index, size_t material) {} // operator ()

  ///
  //
  ///
  T * data() {
    return nullptr;
  } // data

  ///
  //
  ///
  void commit() {} // commit

private:
}; // struct sparse_mutator_t

//----------------------------------------------------------------------------//
// Sparse handle.
//----------------------------------------------------------------------------//

template<typename T>
struct sparse_handle_t {}; // struct sparse_handle_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

///
// FIXME: Sparse storage type.
///
template<>
struct storage_class__<sparse> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  template<typename T>
  using mutator_t = sparse_mutator_t<T>;

  template<typename T>
  using handle_t = sparse_handle_t<T>;

  ///
  //
  ///
  template<typename T, size_t NS>
  static mutator_t<T> get_mutator(
      const data_client_t & data_client,
      const utils::const_string_t & key,
      size_t slots,
      size_t version) {
    return {};
  } // get_accessor

  ///
  //
  ///
  template<typename T, size_t NS>
  static handle_t<T> get_handle(
      const data_client_t & data_client,
      const utils::const_string_t & key,
      size_t version) {
    return {};
  } // get_handle

}; // struct storage_class__

} // namespace legion
} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
