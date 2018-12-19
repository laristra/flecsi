/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include <flecsi/data/common/registration_wrapper.h>
#include <flecsi/data/mpi/color.h>
#include <flecsi/data/mpi/dense.h>
#include <flecsi/data/mpi/global.h>
#include <flecsi/data/mpi/sparse.h>
#include <flecsi/data/storage.h>
//#include <flecsi/data/mpi/scoped.h>
//#include <flecsi/data/mpi/tuple.h>

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct mpi_data_policy_t {
  //--------------------------------------------------------------------------//
  //! The storage_class_u type determines the underlying storage mechanism
  //! for the backend runtime.
  //--------------------------------------------------------------------------//

  template<size_t STORAGE_CLASS>
  using storage_class_u = mpi::storage_class_u<STORAGE_CLASS>;

}; // class mpi_data_policy_t

} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
