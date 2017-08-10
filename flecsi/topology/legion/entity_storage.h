/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_legion_entity_storage_policy_h
#define flecsi_topology_legion_entity_storage_policy_h

#include "flecsi/topology/legion/array_buffer.h"

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {
namespace topology {

template<typename T>
using entity_storage__ = array_buffer__<T>;

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_legion_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

