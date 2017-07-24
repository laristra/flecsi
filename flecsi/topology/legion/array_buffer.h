/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_legion_array_buffer_h
#define flecsi_topology_legion_array_buffer_h

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {
namespace topology {

template<size_t, typename>
class domain_entity;

template<typename item_t>
struct array_buffer_type__{
  using type = item_t;
};

template<size_t M, typename E>
struct array_buffer_type__<domain_entity<M, E>>{
  using type = E*;
};

template<typename T>
class array_buffer__{
public:
  using item_t = typename array_buffer_type__<T>::type;

  array_buffer__(){}

  array_buffer__(
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

  size_t
  size()
  const
  {
    return size_;
  } // size

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

#endif // flecsi_topology_legion_array_buffer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

