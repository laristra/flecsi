/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_common_data_hash_h
#define flecsi_data_common_data_hash_h

#include <utility>

///
/// \file
/// \date Initial file creation: Apr 10, 2017
///

namespace flecsi {
namespace data {

///
/// \class data_hash_key_t data_hash.h
/// \brief data_hash_key_t provides...
///
struct data_hash_key_t : public std::pair<size_t, size_t>
{
  data_hash_key_t(const std::pair<size_t, size_t> & key)
    : std::pair<size_t, size_t>(key) {}

  ///
  /// Return the hash of the namespace.
  ///
  const
  size_t 
  namespace_hash()
  const
  {
    return std::get<0>(*this);
  }

  ///
  /// Return the hash of the name.
  ///
  const
  size_t 
  name_hash()
  const
  {
    return std::get<1>(*this);
  }

}; // class data_hash_key_t

struct data_hash_t
{
  using key_t = std::pair<size_t, size_t>;

  static
  data_hash_key_t
  make_key(
    size_t namespace_hash,
    size_t name_hash
  )
  {
    return std::make_pair(namespace_hash, name_hash);
  } // make_key

  std::size_t
  operator () (
    const data_hash_key_t & key
  )
  const
  {
    return ((key.namespace_hash() << 32) ^ key.name_hash());
  } // operator ()

  bool
  operator () (
    const data_hash_key_t & a,
    const data_hash_key_t & b
  )
  const
  {
    return a.namespace_hash() == b.namespace_hash() &&
      a.name_hash() == b.name_hash();
  } // operator ()


}; // struct data_hash_t


} // namespace data
} // namespace flecsi

#endif // flecsi_data_common_data_hash_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
