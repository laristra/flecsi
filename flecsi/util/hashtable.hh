
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

#include "flecsi/flog.hh"
#include <flecsi/util/array_ref.hh>
#include <utility>

namespace flecsi {
namespace util {

template<class KEY, class TYPE, class HASH = std::hash<KEY>>
class hashtable;

/**
 * @brief hashtableIterator to iterate on the hashtable.
 * It iterator on the element defined in the hashtable,
 * if an element key is zero.
 */
template<class KEY, class TYPE, class HASH>
class hashtableIterator
{
  using ht_t = hashtable<KEY, TYPE, HASH>;
  using ht_type_t = typename ht_t::pair_t;

private:
  ht_type_t * ptr_;
  const ht_t * h_;

  friend class hashtable<KEY, TYPE, HASH>;
  hashtableIterator(ht_type_t * p, const ht_t * h) : ptr_(p), h_(h) {}

public:
  ht_type_t & operator*() const {
    return *ptr_;
  }

  hashtableIterator & operator++() {
    do {
      ++ptr_;
    } while(ptr_ < h_->end().ptr_ && ptr_->first == key_t{});
    return *this;
  }

  bool operator==(const hashtableIterator & iter) const {
    return this->ptr_ == iter.ptr_;
  }

  bool operator!=(const hashtableIterator & iter) const {
    return this->ptr_ != iter.ptr_;
  }

  constexpr ht_type_t * operator->() const {
    return ptr_;
  }
};

/**
 * @brief hashtable implementation.
 * This hashtable is based on span as 1D array and implement the method of
 * displacement when there is a collision, placing the element at a
 * distance of "modulo_".
 * In this current implementation the neutral key is the default constructor
 * of the object :key_t{}, 0 for numeric types.
 */
template<class KEY, class TYPE, class HASH>
struct hashtable {

public:
  // Key use for search in the table
  using key_t = KEY;
  // Type stored with the key
  using type_t = TYPE;
  using pair_t = std::pair<key_t, type_t>;
  // Hasher
  using hash_f = HASH;

  // Iterator
  using pointer = pair_t *;
  using iterator = hashtableIterator<KEY, TYPE, HASH>;
  using const_iterator = const pointer;

private:
  std::size_t nelements_;
  constexpr static std::size_t modulo_ = 334214459;
  util::span<pair_t> span_;

  // Max number of search before crash
  constexpr static std::size_t max_find_ = 10;

public:
  hashtable(const util::span<pair_t> & span) {
    span_ = span;
    nelements_ = 0;
    for(auto & a : *this) {
      ++nelements_;
    }
  }

  /**
   * @brief Find a value in the hashtable
   * While the value or a null key is not found we keep looping
   */
  iterator find(const key_t & key) {
    std::size_t h = hash_f()(key) % span_.size();
    pointer ptr = span_.data() + h;
    std::size_t iter = 0;
    while(ptr->first != key && ptr->first != key_t{} && iter != max_find_) {
      h = (h + modulo_) % span_.size();
      ptr = span_.data() + h;
      ++iter;
    }
    if(ptr->first != key) {
      return end();
    }
    return iterator(ptr, this);
  }

  /**
   * @brief Insert an object in the hash map at a defined position
   * This function tries to find the first available position in case of
   * conflict using modulo method.
   */
  template<typename... ARGS>
  iterator insert(const key_t & key, ARGS &&... args) {
    std::size_t h = hash_f()(key) % span_.size();
    pointer ptr = span_.data() + h;
    std::size_t iter = 0;
    while(ptr->first != key && ptr->first != key_t{} && iter != max_find_) {
      h = (h + modulo_) % span_.size();
      ptr = span_.data() + h;
      ++iter;
    }

    if(iter == max_find_) {
      flog(error) << "Max iteration reached, couldn't insert element: " << key
                  << std::endl;
      return end();
    }
    ++nelements_;
    ptr = new(ptr) pair_t(key, {std::forward<ARGS>(args)...});
    return iterator(ptr, this);
  }

  // Clear all keys
  void clear() {
    nelements_ = 0;
    for(auto & a : *this) {
      a.first = key_t{};
    }
  }

  constexpr iterator begin() const noexcept {
    auto it = iterator(span_.begin(), this);
    if(it->first == key_t{})
      ++it;
    return it;
  }

  constexpr iterator end() const noexcept {
    return iterator(span_.end(), this);
  }

  constexpr const_iterator cbegin() const noexcept {
    return begin();
  }

  constexpr const_iterator cend() const noexcept {
    return end();
  }

  constexpr std::size_t size() const noexcept {
    return nelements_;
  }

  constexpr bool empty() const noexcept {
    return begin() == end();
  }

}; // class hashtable

} // namespace util
} // namespace flecsi
