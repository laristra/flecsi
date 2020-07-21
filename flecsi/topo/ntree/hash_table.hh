
/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //
   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/run/context.hh"
#include "flecsi/topo/ntree/types.hh"

namespace flecsi {
namespace topo {

//----------------------------------------------------------------------------//
//! Hash table class for the tree topology. This is a generic representation
//! based on an index space implementing find insert and find functions
//----------------------------------------------------------------------------//

template<class KEY, class TYPE>
struct hash_table {

private:
  using id_t = util::id_t;
  using key_t = KEY;
  using type_t = TYPE;

  static const size_t modulo_ = 1234;

public:
  /**
   * @brief Find a value in the hashtable
   * While the value or a null key is not found we keep looping
   */
  template<const auto & F, std::size_t Priv>
  static type_t * find(const data::accessor_member<F, Priv> & acc,
    const key_t & key) {
    size_t hash_capacity_ = acc.span().size();
    size_t h = hash(key, hash_capacity_);
    type_t * ptr = acc.span().data() + h;
    while(ptr->key() != key && ptr->key() != key_t::null()) {
      h += modulo_;
      h = h >= hash_capacity_ ? h % hash_capacity_ : h;
      ptr = acc.span().data() + h;
    }
    if(ptr->key() != key) {
      return nullptr;
    }
    return ptr;
  }

  /**
   * @brief Insert an object in the hash map at a defined position
   * This function tries to find the first available position in case of
   * conflict using modulo method.
   */
  template<const auto & F, std::size_t Priv, class... ARGS>
  static type_t * insert(const data::accessor_member<F, Priv> & acc,
    const key_t & key,
    ARGS &&... args) {
    size_t hash_capacity_ = acc.span().size();
    size_t h = hash(key, hash_capacity_);
    type_t * ptr = acc.span().data() + h;
    while(ptr->key() != key && ptr->key() != key_t::null()) {
      h += modulo_;
      h = h >= hash_capacity_ ? h % hash_capacity_ : h;
      ptr = acc.span().data() + h;
    }
    auto b = new(ptr) type_t(key, std::forward<ARGS>(args)...);
    return ptr;
  }

  // Clear all keys
  template<const auto & F, std::size_t Priv>
  static void clear(const data::accessor_member<F, Priv> & acc) {
    for(auto a : acc.span()) {
      a.set_key(key_t::null());
    }
  }

  /**
   * @brief the Hash function transforming a key in position in the hash
   * table.
   */
  static size_t hash(const key_t & key, size_t capacity) {
    return key % capacity;
  }
}; // class hash_table

} // namespace topo
} // namespace flecsi
