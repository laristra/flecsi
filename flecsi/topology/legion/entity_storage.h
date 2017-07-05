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

template<size_t, typename>
class domain_entity;

template<typename item_t>
struct entity_storage_type__{
  using type = item_t;
};

template<size_t M, typename E>
struct entity_storage_type__<domain_entity<M, E>>{
  using type = E*;
};

template<typename T>
class entity_storage__{
public:
  using item_t = typename entity_storage_type__<T>::type;

  entity_storage__(){}

  entity_storage__(
    item_t* buf,
    size_t size
  )
  : buf_(buf),
  size_(size){}

  item_t
  operator[](size_t index)
  {
    return buf_ + index;
  }

  const item_t
  operator[](size_t index)
  const
  {
    return buf_ + index;
  }

  item_t
  begin()
  {
    return buf_;
  }

  item_t
  end()
  {
    return buf_ + size_;
  }

  const item_t
  begin()
  const
  {
    return buf_;
  }

  const item_t
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
  set_buffer(item_t buf, size_t size)
  {
    buf_ = buf;
    size_ = size;
  }

  item_t
  buffer()
  {
    return buf_;
  }

private:
  item_t buf_;
  size_t size_;  
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_legion_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

