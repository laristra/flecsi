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
using topology_storage__ = array_buffer__<T>;

class offset_storage_{
public:

  size_t
  get(size_t i)
  const
  {
    return s_[i].x[0];
  }

  size_t
  next(size_t i)
  const
  {
    return s_[i].x[0] + s_[i].x[1];
  }

  size_t
  count(size_t i)
  const
  {
    return s_[i].x[1];
  }

  auto&
  storage()
  {
    return s_;
  }  

private:
  topology_storage__<LegionRuntime::Arrays::Point<2>> s_;  
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_legion_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

