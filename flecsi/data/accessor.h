/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_accessor_h
#define flecsi_accessor_h

#include "flecsi/data/data_constants.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 28, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

struct accessor_base_t {};

template<
  data::storage_label_type_t,
  typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS
>
struct accessor_type__{};

template<
  data::storage_label_type_t STORAGE_TYPE,
  typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS = 0,
  size_t GHOST_PERMISSIONS = 0
>
struct accessor__ :
public accessor_type__<
  STORAGE_TYPE,
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS
>::type
{
  static constexpr data::storage_label_type_t storage_type = STORAGE_TYPE;
};

} // namespace flecsi

#endif // flecsi_accessor_h

/*~-------------------------------------------------------------------------~-*
*~-------------------------------------------------------------------------~-*/
