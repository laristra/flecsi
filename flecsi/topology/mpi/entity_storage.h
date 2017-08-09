/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_mpi_entity_storage_policy_h
#define flecsi_topology_mpi_entity_storage_policy_h

#include "flecsi/topology/common/array_buffer.h"

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {
namespace topology {

#if 0
template<typename T>
class topology_storage__{
public:
  topology_storage__(){}

  topology_storage__(
    T* buf,
    size_t size
  )
  : buf_(buf),
  size_(size){}

  T
  operator[](size_t index)
  {
    return buf_ + index;
  }

  const T
  operator[](size_t index)
  const
  {
    return buf_ + index;
  }

  T
  begin()
  {
    return buf_;
  }

  T
  end()
  {
    return buf_ + size_;
  }

  const T
  begin()
  const
  {
    return buf_;
  }

  const T
  end()
  const
  {
    return buf_ + size_;
  }

  template<
    typename ... Args
  >
  void insert(Args && ... args){
    assert(false && "unimplemented");
  }

  template<
    typename ... Args
  >
  void push_back(Args && ... args){
    assert(false && "unimplemented");
  }

  void
  clear()
  {
    assert(false && "unimplemented");
  }

private:
  T buf_;
  size_t size_;  
};
#endif

template<typename T>
using topology_storage__ = array_buffer__<T>;

class offset_storage_ {
public:
  const size_t
  operator[](size_t i)
  const
  {
    return start_[i];
  }

  void
  set(size_t i, size_t offset)
  {
    if (i > 0) {
      start_[i] = offset;
      count_[i] = offset - start_[i-1];
    } else {
      start_[0] = 0;
    }
  }

  size_t
  next(size_t i)
  const
  {
    return start_[i] + count_[i];
  }

  std::pair<size_t, size_t>
  range(size_t i)
  const
  {
    size_t begin = start_[i];
    return {begin, begin + count_[i]};
  }

  size_t
  count(size_t i)
  {
    return count_[i];
  }

  void
  clear()
  {
    start_.clear();
    count_.clear();
  }

  auto&
  storage()
  {
    // TODO: which on to return?
    return start_;
  }

  void
  push_back(size_t offset)
  {
    size_t size = start_.size();

    if(size > 0){
      start_[size] = offset;
      count_[size-1] = offset - start_[size-1];
    } else{
      start_[0] = 0;
    }

    start_.pushed();
  }

  void
  resize(size_t n)
  {
    start_.resize(n - 1);
  }

  size_t
  size()
  const
  {
    return start_.size();
  }

private:
  topology_storage__ <size_t> start_;
  topology_storage__ <size_t> count_;
};
} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_mpi_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

