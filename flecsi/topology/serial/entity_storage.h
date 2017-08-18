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
using topology_storage__ = std::vector<T>;

class offset_storage_{
public:

  const size_t
  operator[](size_t i)
  const
  {
    return s_[i];
  }

  void
  set(size_t i, size_t offset)
  {
    s_[i] = offset;
  }

  size_t
  next(size_t i)
  const
  {
    return s_[i + 1];
  }

  std::pair<size_t, size_t>
  range(size_t i)
  const
  {
    return {s_[i], s_[i + 1]};
  }

  size_t
  count(size_t i)
  const
  {
    return s_[i + 1] - s_[i];
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
    s_.push_back(offset);
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
  std::vector<size_t> s_;  
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_serial_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

