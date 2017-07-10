/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_legion_entity_storage_policy_h
#define flecsi_topology_legion_entity_storage_policy_h

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {
namespace topology {

template<typename T>
class entity_storage__{
public:
  entity_storage__(){}

  entity_storage__(
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
  void push_back(Args && ... args){}

  void
  clear()
  {
    assert(false && "unimplemented");
  }

  void
  set_buffer(T buf, size_t size)
  {
    buf_ = buf;
    size_ = size;
  }

  T
  buffer()
  {
    return buf_;
  }

private:
  T buf_;
  size_t size_;  
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_legion_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

