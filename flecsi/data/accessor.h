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
struct accessor__ : public accessor_base_t{};

template<
  typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS
>
struct accessor__<
  data::storage_label_type_t::base,
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS
>
{
  
};

} // namespace flecsi

#endif // flecsi_accessor_h

/*~-------------------------------------------------------------------------~-*
*~-------------------------------------------------------------------------~-*/
