/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 03, 2017
//----------------------------------------------------------------------------//

#ifndef flecsi_topology_set_storage_h
#define flecsi_topology_set_storage_h

#include "flecsi/runtime/flecsi_runtime_set_topology_policy.h"

namespace flecsi{
namespace topology{

template<
  typename SET_TYPES
>
class set_storage_t : 
  public FLECSI_RUNTIME_SET_TOPOLOGY_STORAGE_POLICY<SET_TYPES>
{};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_set_storage_h

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
