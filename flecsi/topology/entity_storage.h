/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_entity_storage_h
#define flecsi_topology_entity_storage_h

#include "flecsi/runtime/flecsi_runtime_entity_storage_policy.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {

template<typename T>
using entity_storage_t = FLECSI_RUNTIME_ENTITY_STORAGE_TYPE<T>;

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_entity_storage_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
