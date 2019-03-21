#pragma omp

#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/topology/index_space.h>
#include <flecsi/topology/types.h>

#include <flecsi/topology/tree/tree_utils.h>

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
//! Hash table class for the tree topology. This is a generic representation
//! based on an index space implementing find insert and find functions
//----------------------------------------------------------------------------//

template<class KEY, class TYPE>
struct hash_table_u {

  using id_t = utils::id_t;
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
      std::cout << "Find collision" << std::endl;
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
      std::cout << "Collision: " << key << std::endl;
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
}; // class hash_table_u

template<class K, class T>
size_t hash_table_u<K, T>::collision = 0;

} // namespace topology
} // namespace flecsi
