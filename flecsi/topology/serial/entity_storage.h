/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_serial_entity_storage_policy_h
#define flecsi_topology_serial_entity_storage_policy_h

#include <vector>
#include <cassert>

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {
namespace topology {

template<typename T>
using entity_storage__ = std::vector<T>;

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_serial_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

