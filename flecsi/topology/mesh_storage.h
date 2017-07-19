/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mesh_storage_h
#define flecsi_mesh_storage_h

#include "flecsi/runtime/flecsi_runtime_topology_policy.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {

template <size_t ND, size_t NM>
class mesh_storage_t : public FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY<ND, NM>
{};

} // namespace topology
} // namespace flecsi

#endif // flecsi_mesh_storage_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
