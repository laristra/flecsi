/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_storage_class_h
#define flecsi_data_storage_class_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 15, 2016
//----------------------------------------------------------------------------//

#ifndef POLICY_NAMESPACE
  #error "You must define a data policy namespace before including this file."
#endif

#include "flecsi/data/data_constants.h"

namespace flecsi {
namespace data {
namespace POLICY_NAMESPACE {

  ///
  /// \struct storage_class__
  ///
  /// \tparam T Specialization parameter.
  /// \tparam ST Data store type.
  /// \tparam MD Metadata type.
  ///
  template<size_t STORAGE_CLASS>
  struct storage_class__ {};

} // namespace POLICY_NAMESPACE
} // namespace data
} // namespace flecsi

#endif // flecsi_data_storage_class_h

/*~-------------------------------------------------------------------------~-*
*~-------------------------------------------------------------------------~-*/
