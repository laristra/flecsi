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

  const size_t
  operator[](size_t i)
  const
  {
    size_t size = s_.size();
    return i == size ? s_[size - 1].x[0] + s_[size - 1].x[1] : s_[i].x[0];
  }

  void
  set(size_t i, size_t offset)
  {
    size_t size = s_.size();
    if(i == size){
      s_[size - 1].x[1] = offset - s_[size - 1].x[0];
      return;
    }

    if(i > 0){
      s_[i].x[0] = offset;
      s_[i - 1].x[1] = offset - s_[i - 1].x[0];
    }
    else{
      s_[0].x[0] = 0;
    }    
  }

  std::pair<size_t, size_t>
  range(size_t i)
  const
  {
    size_t begin = s_[i].x[0];
    return {begin, begin + s_[i].x[1]};
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
  init()
  {
    assert(s_.empty());
    s_[0].x[0] = 0;
    s_.pushed();
  }

  void
  push_back(size_t offset)
  {
    assert(!s_.empty());

    size_t size = s_.size();
    size_t capacity = s_.capacity();

    if(size == capacity){
      s_[size - 1].x[1] = offset - s_[size - 1].x[0];
      return;
    }

    s_[size].x[0] = offset;
    s_[size - 1].x[1] = offset - s_[size - 1].x[0];

    s_.pushed();
  }

  void
  resize(size_t n)
  {
    s_.resize(n - 1);
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

