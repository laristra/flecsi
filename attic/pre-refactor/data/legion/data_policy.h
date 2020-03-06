/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include <flecsi/data/storage.h>

#include <flecsi/data/legion/color.h>
#include <flecsi/data/legion/dense.h>
#include <flecsi/data/legion/global.h>
#include <flecsi/data/legion/scoped.h>
#include <flecsi/data/legion/sparse.h>
#include <flecsi/data/legion/tuple.h>

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! The legion data policy defines types that are specific to the Legion
//! runtime.
//----------------------------------------------------------------------------//

struct legion_data_policy_t {

  //--------------------------------------------------------------------------//
  //! The storage_class_u type determines the underlying storage mechanism
  //! for the backend runtime.
  //--------------------------------------------------------------------------//

  template<size_t STORAGE_CLASS>
  using storage_class_u = legion::storage_class_u<STORAGE_CLASS>;

}; // class legion_data_policy_t

} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
