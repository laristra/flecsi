/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 03, 2017
//----------------------------------------------------------------------------//

#ifndef flecsi_topology_set_topology_h
#define flecsi_topology_set_topology_h

#include "flecsi/topology/set_types.h"
#include "flecsi/topology/set_utils.h"
#include "flecsi/topology/set_storage.h"

namespace flecsi{
namespace topology{

template<
  typename SET_TYPES
>
class set_topology_t : 
public set_topology_base_t<set_storage_t<SET_TYPES>>
{
  static const size_t num_index_spaces = 
    std::tuple_size<typename SET_TYPES::entity_types_t>::value;
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_set_topology_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
