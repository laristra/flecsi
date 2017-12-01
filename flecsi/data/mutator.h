/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mutator_h
#define flecsi_mutator_h

#include "flecsi/data/data_constants.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 28, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

struct mutator_base_t {};

template<
  data::storage_label_type_t,
  typename T
>
struct mutator__ : public mutator_base_t{};

template<
  typename T
>
struct mutator__<
  data::base,
  T
>
{
  
};

} // namespace flecsi

#endif // flecsi_mutator_h

/*~-------------------------------------------------------------------------~-*
*~-------------------------------------------------------------------------~-*/
