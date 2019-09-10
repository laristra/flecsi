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

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 17, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {
namespace legion {

///
// FIXME: Scoped storage type.
///
template<>
struct storage_class_u<scoped> {

  ///
  //
  ///
  struct scoped_handle_t {}; // struct scoped_handle_t

}; // struct storage_class_u

} // namespace legion
} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
