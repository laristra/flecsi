/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_class.h!!!
// Using this approach allows us to have only one storage_class_u
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include <flecsi/data/storage_class.h>
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include <flecsi/utils/const_string.h>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 17, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {
namespace legion {

///
// FIXME: Tuple storage type.
///
template<>
struct storage_class_u<tuple> {

  struct tuple_handle_t {}; // struct tuple_handle_t

  ///
  //
  ///
  template<typename T, size_t NS>
  static tuple_handle_t
  get_handle(uintptr_t runtime_namespace, const utils::const_string_t & key) {
    return {};
  } // get_handle

}; // struct storage_class_u

} // namespace legion
} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
