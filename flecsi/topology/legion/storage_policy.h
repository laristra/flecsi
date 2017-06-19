/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_legion_topology_storage_policy_h
#define flecsi_topology_legion_topology_storage_policy_h

#include <legion.h>
#include <legion_stl.h>

#include "flecsi/topology/mesh_storage.h"

#include <array>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <vector>

#include "flecsi/data/data_client.h"
#include "flecsi/topology/mesh_utils.h"
#include "flecsi/utils/array_ref.h"
#include "flecsi/utils/reorder.h"
#include "flecsi/topology/index_space.h"

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {
namespace topology {

///
/// \class legion_data_handle_policy_t data_handle_policy.h
/// \brief legion_data_handle_policy_t provides...
///

template <size_t D, size_t NM>
struct legion_topology_storage_policy_t
{
  using index_spaces_t = 
    std::array<index_space<mesh_entity_base_*, true, true, true>, D + 1>;

  // array of array of domain_connectivity
  std::array<std::array<domain_connectivity<D>, NM>, NM> topology;

  std::array<index_spaces_t, NM> index_spaces;
}; // class legion_topology_storage_policy_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_legion_topology_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

