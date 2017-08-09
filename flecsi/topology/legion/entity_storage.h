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
  operator[](size_t i)
  const
  {
    return s_[i].x[0];
  }

  auto&
  operator[](size_t i)
  {
    return s_[i].x[0];
  }

  size_t
  next(size_t i)
  const
  {
    return s_[i].x[0] + s_[i].x[1];
  }

  std::pair<size_t, size_t>
  range(size_t i)
  const
  {
    size_t begin = s_[i].x[0];
    return {begin, begin + s_[i].x[1]};
  }

  size_t
  count(size_t i)
  const
  {
    return s_[i].x[1];
  }

  void
  clear()
  {
    s_.clear();
  }

  auto&
  storage()
  {
    return s_;
  }

  void
  push_back(size_t offset)
  {
    size_t size = s_.size();

    if(size > 0){
      s_[size].x[0] = offset;
      s_[size - 1].x[1] = offset - s_[size - 1].x[0];
    }
    else{
      s_[0].x[0] = 0;
    }

    s_.pushed();
  }

  void
  resize(size_t n)
  {
    s_.resize(n);
  }

  size_t
  size()
  const
  {
    return s_.size();
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

