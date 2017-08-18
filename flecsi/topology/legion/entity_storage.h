/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_legion_entity_storage_policy_h
#define flecsi_topology_legion_entity_storage_policy_h

#include "flecsi/topology/common/array_buffer.h"

#include "flecsi/utils/offset.h"

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

  using offset_t = utils::offset_t;

  const offset_t
  operator[](size_t i)
  const
  {
    return s_[i];
  }

  void
  add_count(uint32_t count)
  {
    offset_t o(start_, count);
    s_.push_back(o);
    start_ += count;
  }

  void
  add_end(size_t end)
  {
    assert(end > start_);
    add_count(end - start_);
  }

  std::pair<size_t, size_t>
  range(size_t i)
  const
  {
    return s_[i].range();
  }

  void
  clear()
  {
    s_.clear();
    start_ = 0;
  }

  auto&
  storage()
  {
    return s_;
  }

  size_t
  size()
  const
  {
    return s_.size();
  }

private:
  topology_storage__<offset_t> s_;
  size_t start_ = 0;  
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_legion_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
