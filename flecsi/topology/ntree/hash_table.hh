
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

#pragma omp

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/runtime/context.hh"
#include <flecsi/topology/ntree/types.hh>

namespace flecsi {
namespace topo {

//----------------------------------------------------------------------------//
//! Hash table class for the tree topology. This is a generic representation
//! based on an index space implementing find insert and find functions
//----------------------------------------------------------------------------//

template<class KEY, class TYPE>
struct hash_table {

  using id_t = util::id_t;
  using key_t = KEY;
  using type_t = TYPE;

  // The number of nits used for the hash table
  static constexpr size_t hash_bits_ = 22;
  // The capacity of the hash table
  static constexpr size_t hash_capacity_ = 1 << hash_bits_;
  // The simple mask used in this function
  static constexpr size_t hash_mask_ = (1 << hash_bits_) - 1;
  static constexpr size_t modulo = 5;

  static size_t collision; // Collision counter

  /**
   * @brief Find a value in the hashtable
   * While the value or a null key is not found we keep looping
   */
  template<typename S>
  static type_t * find(S & index_space, key_t key) {
    size_t h = hash(key);
    type_t * ptr = index_space.storage()->begin() + h;
    while(ptr->key() != key && ptr->key() != key_t::null()) {
      h += modulo;
      h = h >= hash_capacity_ ? h % hash_capacity_ : h;
      ptr = index_space.storage()->begin() + h;
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
  template<typename S, class... ARGS>
  static type_t * insert(S & index_space, const key_t & key, ARGS &&... args) {
    size_t h = hash(key);
    type_t * ptr = index_space.storage()->begin() + h;
    while(ptr->key() != key && ptr->key() != key_t::null()) {
      h += modulo;
      h = h >= hash_capacity_ ? h % hash_capacity_ : h;
      ptr = index_space.storage()->begin() + h;
      ++collision;
    }
    auto b = new(ptr) type_t(key, std::forward<ARGS>(args)...);
    return ptr;
  }

  /**
   * @brief the Hash function transforming a key in position in the hash
   * table.
   */
  static size_t hash(const key_t & key) {
    return key & hash_mask_;
  }
}; // class hash_table

template<class K, class T>
size_t hash_table<K, T>::collision = 0;

} // namespace topo
} // namespace flecsi
